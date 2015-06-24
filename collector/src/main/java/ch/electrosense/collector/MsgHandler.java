package ch.electrosense.collector;

import ch.electrosense.avro.v1.Sample;

/**
 * MsgHandler process messages read from the TCP socket.
 * All implementing classes MUST be thread-safe!
 * 
 * @author Markus Fuchs <fuchs@sero-systems.de>
 */
public interface MsgHandler {
	/**
	 * called by the ConnectionHandler for each valid Sample
	 * @param rawMsg the byte[] representation of the Sample received
	 * @throws ProcessingException in case something goes wrong while processing
	 * the sample. As a result, all connections will be closed and the collector
	 * is shut down.
	 */
	public void processRaw(byte[] rawMsg) throws ProcessingException;
	
	/**
	 * called by the ConnectionHandler for each valid Sample
	 * @param Sample decoded sample
	 * @throws ProcessingException in case something goes wrong while processing
	 * the sample. As a result, all connections will be closed and the collector
	 * is shut down.
	 */
	public void processDecoded(Sample s) throws ProcessingException;
	
	/**
	 * Free resources occupied by the message handler. This method is called
	 * on shutdown.
	 */
	public void close();
}
