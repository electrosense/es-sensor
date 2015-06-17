package ch.electrosense.collector;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.HashMap;
import java.util.Map;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import ch.electrosense.avro.v1.Sample;

/**
 * A MsgHandler that opens a text file for each sensor id and writes out each
 * raw message in its JSON representation.
 * 
 * @author Markus Fuchs <fuchs@sero-systems.de>
 */
public class FileRawMsgHandler implements MsgHandler {
	Logger logger = LoggerFactory.getLogger(FileRawMsgHandler.class);

	File root;
	Map<Long, PrintWriter> sensor2Writer;

	/**
	 * 
	 * @param path
	 *            a directory to store files for each sensor. The directory must
	 *            exist. Files will be overwritten if they exist!
	 * @throws IOException
	 */
	public FileRawMsgHandler(String path) throws IOException {
		root = new File(path);
		if (!root.exists() || root.isFile()) {
			throw new IOException(
					"Ensure that output path is a directory and it already exists. Given: "
							+ path);
		}
		logger.info("Writing files to " + root.getAbsolutePath());
		sensor2Writer = new HashMap<Long, PrintWriter>();
	}

	/**
	 * not implemented
	 */
	@Override
	public synchronized void processRaw(byte[] rawMsg) {
		// do nothing
	}

	/**
	 * writes the sample into a text file 'sensorid.raw' where sensorid is
	 * inferred from the Sample
	 */
	@Override
	public synchronized void processDecoded(Sample s) {
		if (s == null)
			return;
		Long id = 0l;
		if (s.getSenId() == null)
			logger.warn("Sensor has null SenId");
		else
			id = s.getSenId();

		PrintWriter writer = sensor2Writer.get(id);
		// create and open new writer if not yet exists
		if (writer == null) {
			File outFile = null;
			try {
				outFile = new File(root, id.toString() + ".raw");
				writer = new PrintWriter(outFile);
				sensor2Writer.put(id, writer);
			} catch (FileNotFoundException e) {
				logger.error("Failed to create output file: " + outFile);
				System.exit(1);
			}
		}
		writer.println(s.toString());
	}

	/**
	 * flush and close all open files
	 */
	@Override
	public synchronized void close() {
		logger.info("Closing raw files...");
		for (PrintWriter w : sensor2Writer.values()) {
			w.flush();
			w.close();
		}
	}

}
