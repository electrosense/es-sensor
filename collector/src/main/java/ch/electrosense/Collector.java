package ch.electrosense;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;
import java.util.ArrayList;

import javax.net.ServerSocketFactory;
import javax.net.ssl.SSLServerSocket;
import javax.net.ssl.SSLServerSocketFactory;

import org.apache.commons.cli.CommandLine;
import org.apache.commons.cli.CommandLineParser;
import org.apache.commons.cli.DefaultParser;
import org.apache.commons.cli.HelpFormatter;
import org.apache.commons.cli.Option;
import org.apache.commons.cli.Options;
import org.apache.commons.cli.ParseException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import ch.electrosense.collector.ConnectionHandler;
import ch.electrosense.collector.FileRawMsgHandler;
import ch.electrosense.collector.KafkaMsgHandler;
import ch.electrosense.collector.MsgHandler;

/**
 * Collector main class. Listens for TLS connections of sensors and starts
 * ConnectionHandler for each incoming connection.
 * 
 * @author Markus Fuchs <fuchs@sero-systems.de>
 */
public class Collector extends Thread {
	static Logger logger = LoggerFactory.getLogger(Collector.class);

	public static final int DEFAULT_LISTEN_PORT = 5000;

	int port;
	ArrayList<ConnectionHandler> threads;
	ArrayList<MsgHandler> msgHandler;
	ServerSocket serverSocket;
	boolean sslEnabled;

	private volatile boolean running = true;

	/**
	 * 
	 * @param port
	 *            port to listen to; use default if null
	 * @param sslEnabled
	 *            whether to provide TLS sockets or not; TLS and non-TLS are
	 *            mutually exclusive, i.e. we listen only for secured
	 *            connections or we don't
	 */
	public Collector(Integer port, boolean sslEnabled) {
		if (port == null)
			port = DEFAULT_LISTEN_PORT;
		this.port = port;

		threads = new ArrayList<ConnectionHandler>();
		msgHandler = new ArrayList<MsgHandler>();
		this.sslEnabled = sslEnabled;
	}

	/**
	 * Add a messages handler to the collector instance. Before the thread is
	 * started there must be at least one!
	 * 
	 * @param h
	 *            a message handler which is called for each incoming message
	 */
	public synchronized void addMsgHandler(MsgHandler h) {
		msgHandler.add(h);
	}

	public void run() {
		if (msgHandler.size() < 1)
			throw new RuntimeException("No MsgHandler set before start");

		ServerSocketFactory socketFactory = null;
		if (this.sslEnabled) {
			socketFactory = SSLServerSocketFactory.getDefault();
			try {
				serverSocket = (SSLServerSocket) socketFactory
						.createServerSocket(port);
				((SSLServerSocket) serverSocket).setNeedClientAuth(true);
				// This would also accept clients without a cert:
				// serverSocket.setWantClientAuth(true);
			} catch (IOException e) {
				logger.error("Could not set up TLS socket", e);
				System.exit(1);
			}
			// TODO restrict cipher suites
			// String [] suites = new String [] { "SSL_RSA_WITH_DES_CBC_SHA"
			// };//If
			// you wish to limit ciphers
			// String [] protocols = new String [] { "SSLv3" };
			// serverSocket.setEnabledProtocols(protocols);
			// serverSocket.setEnabledCipherSuites(suites);
		} else {
			logger.warn("TLS disabled: Accepting non-encrypted and non-authenticated connections");
			socketFactory = ServerSocketFactory.getDefault();
			try {
				serverSocket = socketFactory.createServerSocket(port);
			} catch (IOException e) {
				logger.error("Could not set up socket", e);
				System.exit(1);
			}
		}

		logger.info("Starting collector, listening on "
				+ serverSocket.getInetAddress().toString());
		while (running) {
			Socket clientSocket = null;
			try {
				clientSocket = serverSocket.accept();
			} catch (SocketException es) {
				// close() called on this socket will cause this exception
				break;
			} catch (IOException e) {
				logger.error("Error initiating connection", e);
				continue;
			}
			ConnectionHandler handler = new ConnectionHandler(clientSocket,
					this.msgHandler);
			threads.add(handler);
			handler.start();

			// clean up dead threads
			ArrayList<Integer> dead = new ArrayList<Integer>();
			for (int i = 0; i < threads.size(); i++) {
				ConnectionHandler h = threads.get(i);
				if (h == null || !h.isAlive()) {
					logger.debug("Removing dead Thread " + h.getName());
					dead.add(i);
				}
			}
			for (Integer i : dead)
				threads.remove(i.intValue());
		}
	}

	/**
	 * Call this method to gracefully shutdown the collector thread. It will
	 * disconnect all clients and free resources of the message handlers.
	 */
	public synchronized void shutdown() {
		logger.info("Collector shutting down...");
		logger.info("Disconnecting clients...");
		for (ConnectionHandler c : threads) {
			try {
				if (c == null || !c.isAlive())
					continue;
				c.disconnect();
				c.join(3000);
			} catch (InterruptedException e) {
				logger.info("Interrupted while disconnecting client");
			}
		}
		logger.info("Closing server socket...");
		running = false;
		try {
			if (serverSocket != null)
				serverSocket.close();
		} catch (IOException e) {
			// ignore
		}
		for (MsgHandler h : msgHandler) {
			h.close();
		}
		logger.info("Done.");
	}

	/**
	 * print help for CLI options
	 */
	public static <T> void printHelp(Options opts, Class<T> clazz) {
		HelpFormatter help = new HelpFormatter();
		help.printHelp(80, clazz.getName(), "Options", opts, "");
	}

	public static void main(String[] args) {
		// CLI options
		final String OPT_PORT = "p";
		final String OPT_KAFKA_DEFAULT = "k";
		final String OPT_KAFKA_HOSTS = "m";
		final String OPT_KAFKA_TOPIC = "t";
		final String OPT_KEYSTORE = "K";
		final String OPT_TRUSTSTORE = "T";
		final String OPT_PASS = "P";
		final String OPT_RAW_TEXT_OUT = "d";

		Options opts = new Options();
		opts.addOption(Option.builder(OPT_PORT).required(false).argName("port")
				.desc("(optional) Port to listen on, default: 5000").hasArg()
				.build());
		opts.addOption(Option
				.builder(OPT_TRUSTSTORE)
				.required(false)
				.argName("path")
				.desc("(optional) Path to Java truststore that contains CA cert")
				.hasArg().build());
		opts.addOption(Option
				.builder(OPT_KEYSTORE)
				.required(false)
				.argName("path")
				.desc("(optional) Path to Java keystore that contains the server cert and secret key")
				.hasArg().build());
		opts.addOption(Option
				.builder(OPT_PASS)
				.required(false)
				.desc("(optional) Password for Trust-and Keystore (must be the same for both)")
				.hasArg().build());
		opts.addOption(Option
				.builder(OPT_KAFKA_HOSTS)
				.required(false)
				.desc("(optional) Kafka Hosts. Format <hostname1>:<port1>,...,<hostnameN>:<portN> default: "
						+ KafkaMsgHandler.DEFAULT_KAFKA_HOSTS).hasArg().build());
		opts.addOption(Option
				.builder(OPT_KAFKA_TOPIC)
				.required(false)
				.argName("topic")
				.desc("(optional) Kafka Topic default: "
						+ KafkaMsgHandler.DEFAULT_KAFKA_TOPIC).hasArg().build());
		opts.addOption(Option
				.builder(OPT_KAFKA_DEFAULT)
				.required(false)
				.desc("(optional) Enable Kafka output using default hosts and topic. This is not needed if either -m or -t is present.")
				.hasArg(false).build());
		opts.addOption(Option.builder(OPT_RAW_TEXT_OUT).required(false)
				.argName("path").desc("Enable raw file output for debugging")
				.hasArg(true).build());

		CommandLineParser optParser = new DefaultParser();
		CommandLine line = null;
		try {
			line = optParser.parse(opts, args);
		} catch (ParseException e) {
			printHelp(opts, Collector.class);
			System.exit(1);
		}

		Integer port = null;

		// set kafka props if needed, leave null will result in default values
		// being inserted by KafkaMsgHandler
		String kafkaHosts = null;
		String kafkaTopic = null;
		if (line.hasOption(OPT_PORT))
			port = Integer.parseInt(line.getOptionValue(OPT_PORT));
		if (line.hasOption(OPT_KAFKA_HOSTS))
			kafkaHosts = line.getOptionValue(OPT_KAFKA_HOSTS);
		if (line.hasOption(OPT_KAFKA_TOPIC))
			kafkaTopic = line.getOptionValue(OPT_KAFKA_TOPIC);

		if (!(line.hasOption(OPT_PASS) && line.hasOption(OPT_KEYSTORE) && line
				.hasOption(OPT_TRUSTSTORE))
				&& (line.hasOption(OPT_PASS) || line.hasOption(OPT_KEYSTORE) || line
						.hasOption(OPT_TRUSTSTORE))) {
			System.err
					.println("Either you set all of -P, -K, -T for TLS-only, or none of them to disable TLS.");
			System.exit(1);
		}

		// set keystore/truststore props if needed
		boolean enableSsl = false;
		if (line.hasOption(OPT_PASS)) {
			enableSsl = true;
			System.setProperty("javax.net.ssl.keyStore",
					line.getOptionValue(OPT_KEYSTORE));
			System.setProperty("javax.net.ssl.keyStorePassword",
					line.getOptionValue(OPT_PASS));
			System.setProperty("javax.net.ssl.keyStoreType", "JKS");
			System.setProperty("javax.net.ssl.trustStore",
					line.getOptionValue(OPT_TRUSTSTORE));
			System.setProperty("javax.net.ssl.trustStorePassword",
					line.getOptionValue(OPT_PASS));
			System.setProperty("javax.net.ssl.trustStoreType", "JKS");
		}

		final Collector c = new Collector(port, enableSsl);
		// add message handler
		if (line.hasOption(OPT_KAFKA_DEFAULT) || kafkaHosts != null
				|| kafkaTopic != null) {
			c.addMsgHandler(new KafkaMsgHandler(kafkaHosts, kafkaTopic));
		}
		if (line.hasOption(OPT_RAW_TEXT_OUT)) {
			try {
				c.addMsgHandler(new FileRawMsgHandler(line
						.getOptionValue(OPT_RAW_TEXT_OUT)));
			} catch (IOException e) {
				System.err.println(e.getMessage());
				System.exit(1);
			}
		}
		// START COLLECTOR!
		c.start();

		// gracefully shutdown if Ctrl-C is pressed or in case of a system
		// shutdown
		Runtime.getRuntime().addShutdownHook(new Thread(new Runnable() {

			public void run() {
				c.shutdown();
			}
		}));
	}
}
