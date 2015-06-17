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
	 */
	public void processRaw(byte[] rawMsg);
	
	/**
	 * called by the ConnectionHandler for each valid Sample
	 * @param Sample decoded sample
	 */
	public void processDecoded(Sample s);
	
	/**
	 * Free resources occupied by the message handler
	 */
	public void close();
}
