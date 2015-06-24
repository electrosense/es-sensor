package ch.electrosense.collector;

/**
 * Thrown in MsgHandler if something goes wrong.
 * 
 * @author Markus Fuchs <fuchs@sero-systems.de>
 */
public class ProcessingException extends Exception {
	private static final long serialVersionUID = -1149552607659343959L;

	public ProcessingException(Throwable cause) {
		super(cause);
	}
	
	public ProcessingException(String message) {
		super(message);
	}

	public ProcessingException(String message, Throwable cause) {
		super(message, cause);
	}
}
