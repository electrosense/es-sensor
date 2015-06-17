package ch.electrosense.collector;

import java.io.IOException;
import java.io.InputStream;
import java.net.Socket;
import java.net.SocketException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.Arrays;

import javax.net.ssl.HandshakeCompletedEvent;
import javax.net.ssl.HandshakeCompletedListener;
import javax.net.ssl.SSLSession;
import javax.net.ssl.SSLSocket;

import org.apache.avro.io.BinaryDecoder;
import org.apache.avro.io.DatumReader;
import org.apache.avro.io.DecoderFactory;
import org.apache.avro.specific.SpecificDatumReader;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import ch.electrosense.avro.v1.Sample;
import ch.electrosense.avro.v1.SenConfRec;

/**
 * Manages a single connection to a sensor. Retrieves data from sensor and
 * calls registered message handlers for each syntactically valid sample received.
 * In case of an error, the connection is immediately closed.
 * 
 * @author Markus Fuchs <fuchs@sero-systems.de>
 */
public class ConnectionHandler extends Thread {
	Logger logger = LoggerFactory.getLogger(ConnectionHandler.class);

	private Socket socket;
	private boolean sslEnabled;
	private InputStream inStr;
	private DatumReader<Sample> datumReader;
	private DecoderFactory decoderFactory;
	
	ArrayList<MsgHandler> msgHandler;

	public ConnectionHandler(Socket sock,
			ArrayList<MsgHandler> msgHandler) {
		this.socket = sock;
		logger.info("New connection initiated with "
				+ socket.getRemoteSocketAddress().toString());

		logger = LoggerFactory.getLogger(socket.getRemoteSocketAddress()
				.toString().replace(".", "-"));
		datumReader = new SpecificDatumReader<Sample>(Sample.class);
		decoderFactory = DecoderFactory.get();

		this.msgHandler = msgHandler;
		this.sslEnabled = (sock instanceof SSLSocket);
	}

	/**
	 * convert a byte array to an integer. Interpret the byte array with
	 * Big Endian byte order
	 */
	public static int byteArrayToBeLong(byte[] b) {
		final ByteBuffer bb = ByteBuffer.wrap(b);
		bb.order(ByteOrder.BIG_ENDIAN);
		return bb.getInt();
	}

	/**
	 * Reads a specific number of n bytes from an InputStream. This method will
	 * block until n bytes have been read
	 * 
	 * @param inStr
	 *            the InputStream to read from
	 * @param reuseBuff
	 *            optional byte buffer for reuse. If it is not given, a new one
	 *            is generated
	 * @param length
	 *            the number of bytes to read
	 * @return a byte array containing the result read from the InputStream
	 * @throws IOException
	 *             if an IOException occurs while reading or the end of the
	 *             stream is reached
	 */
	public static byte[] readBytes(InputStream inStr, byte[] reuseBuff,
			int length) throws IOException {
		if (reuseBuff == null)
			reuseBuff = new byte[length];
		if (reuseBuff != null && reuseBuff.length < length)
			throw new RuntimeException(
					String.format(
							"Programming Error. Buffer too small: b.length = %d, length = %d",
							reuseBuff.length, length));

		int read, i = 0;
		while (i < length) {
			read = inStr.read();
			if (read < 0)
				throw new IOException("End of stream is reached");
			reuseBuff[i] = (byte) read;
			i++;
		}
		return reuseBuff;
	}

	@Override
	public void run() {
		SSLSession session = null;
		if (sslEnabled) {
			// check TLS cert
			((SSLSocket) this.socket)
					.addHandshakeCompletedListener(new HandshakeCompletedListener() {

						public void handshakeCompleted(
								HandshakeCompletedEvent com) {
							logger.info("Handshake completed. TLS session is valid: "
									+ com.getSession().isValid());

						}
					});
			try {
				((SSLSocket) this.socket).startHandshake();
			} catch (IOException e1) {
				logger.error("TLS handshake failed");
				logger.debug("Exception", e1);
				disconnect();
				return;
			}
			session = ((SSLSocket) this.socket).getSession();
		}

		try {
			inStr = socket.getInputStream();
		} catch (IOException e) {
			logger.error("Error creating input stream", e);
			disconnect();
			return;
		}

		byte[] sizeBuf = new byte[4];
		byte[] payloadBuf;
		int packetSize;
		int reducedFftSize;
		int payloadSize;
		BinaryDecoder binDec = null;
		Sample sample = null;
		long lastStatTime = System.currentTimeMillis();
		long numRecv = 0;
		long numRecvLastStat = 0;
		try {
			while (socket.isConnected() && !socket.isClosed()
					&& (!sslEnabled || session.isValid())) {
				// read packet size
				try {
					sizeBuf = readBytes(inStr, sizeBuf, 4);
					// byte order conversion: ntohl()
					// network byte order in IP is BE
					packetSize = byteArrayToBeLong(sizeBuf);
					logger.debug("Read packet size " + packetSize);
				} catch (SocketException es) {
					// e.g. if socket closed
					break;
				} catch (IOException e) {
					logger.error("Error reading packet size from client", e);
					break;
				}

				if (packetSize < 1) {
					logger.error("Got invalid packet size");
					break;
				}

				sizeBuf = new byte[4];
				// read reduced_fft_size
				try {
					sizeBuf = readBytes(inStr, sizeBuf, 4);
					reducedFftSize = byteArrayToBeLong(sizeBuf);
					logger.debug("Read reducedFftSize " + reducedFftSize);
				} catch (SocketException es) {
					// e.g. if socket closed
					break;
				} catch (IOException e) {
					logger.error("Error reading reduced fft size from client",
							e);
					break;
				}

				if (reducedFftSize < 1) {
					logger.error("Got invalid reduced fft size");
					break;
				}

				// read payload
				payloadSize = (packetSize + 3) & ~0x03;
				logger.debug("Payload size is " + payloadSize);
				payloadBuf = new byte[(int) payloadSize];
				try {
					payloadBuf = readBytes(inStr, payloadBuf, payloadSize);
				} catch (SocketException es) {
					// e.g. if socket closed
					break;
				} catch (IOException e) {
					logger.error("Error reading payload from client", e);
					break;
				}
				byte[] payloadBufC = Arrays.copyOf(payloadBuf,
						payloadBuf.length);
				try {
					binDec = decoderFactory.binaryDecoder(payloadBuf, binDec);
					sample = datumReader.read(sample, binDec);
				} catch (IOException e) {
					logger.error("Failed to decode sample", e);
					break;
				}
				logger.debug("Received: " + sample.toString());

				// sanity checks on received sample
				SenConfRec senConf = sample.getSenConf();
				if (sample.getSenTime().getTimeSecs() < 1262304000
						|| // 1/1/2010
						senConf.getFFTSize() < 1
						|| senConf.getFFTSize() > 131072
						|| senConf.getAveragingFactor() <= 0
						|| senConf.getFrequencyOverlap() < 0
						|| senConf.getFrequencyOverlap() > 1
						|| senConf.getFrequencyResolution() < 0
						|| senConf.getHoppingStrategy() < 0
						|| senConf.getWindowingFunction() < 0) {
					logger.info("Sample failed sanity check: "
							+ sample.toString());
					break;
				}

				// do sth with the message
				for (MsgHandler h : msgHandler) {
					h.processRaw(payloadBufC);
					h.processDecoded(sample);
				}

				numRecv++;
				numRecvLastStat++;
				long now = System.currentTimeMillis();
				long delta = now - lastStatTime;
				if (delta >= 60000) {
					logger.info(String
							.format("Current message rate: %.1f msg/s. Total received: %d",
									((float) numRecvLastStat) / delta * 1000,
									numRecv));
					numRecvLastStat = 0;
					lastStatTime = now;
				}
			}
			if (sslEnabled)
				session.invalidate();
		} finally { // while(...)
			disconnect();
		}
	}

	/**
	 * close connections and clean up
	 */
	public synchronized void disconnect() {
		try {
			if (!socket.isClosed()) {
				socket.close();
				logger.info("Connection to "
						+ socket.getRemoteSocketAddress().toString()
						+ " closed");
			}
		} catch (Exception e) {
			// ignore
		}
	}
}
