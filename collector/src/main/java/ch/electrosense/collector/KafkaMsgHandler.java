package ch.electrosense.collector;

import java.util.Properties;

import org.apache.kafka.clients.producer.Callback;
import org.apache.kafka.clients.producer.KafkaProducer;
import org.apache.kafka.clients.producer.ProducerConfig;
import org.apache.kafka.clients.producer.ProducerRecord;
import org.apache.kafka.clients.producer.RecordMetadata;
import org.apache.kafka.common.serialization.ByteArraySerializer;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import ch.electrosense.avro.v1.Sample;

/**
 * Message Handler to write samples to Kafka. It is thread-safe like the
 * KafkaProducer itself.
 * 
 * @author Markus Fuchs <fuchs@sero-systems.de>
 *
 */
public class KafkaMsgHandler implements MsgHandler {
	private Logger logger = LoggerFactory.getLogger(KafkaMsgHandler.class);

	public static final String DEFAULT_KAFKA_TOPIC = "rtl-spec.frequency-spectrum-v2";
	public static final String DEFAULT_KAFKA_HOSTS = "kafka2-1.lambda.armasuisse.ch:9092,kafka2-2.lambda.armasuisse.ch:9092,kafka2-3.lambda.armasuisse.ch:9092,kafka2-4.lambda.armasuisse.ch:9092,kafka2-5.lambda.armasuisse.ch:9092,kafka2-6.lambda.armasuisse.ch:9092";

	Properties kafkaConf;
	KafkaProducer<byte[], byte[]> producer;
	private String kafkaTopic;

	/**
	 * 
	 * @param kafkaHosts
	 *            host string in format host1:port1,...hostN:portN. Uses
	 *            DEFAULT_KAFKA_HOSTS if set to null
	 * @param kafkaTopic
	 *            the topic to write into. Uses DEFAULT_KAFKA_TOPIC if set to
	 *            null
	 */
	public KafkaMsgHandler(String kafkaHosts, String kafkaTopic) {
		if (kafkaHosts == null)
			kafkaHosts = DEFAULT_KAFKA_HOSTS;
		if (kafkaTopic == null)
			kafkaTopic = DEFAULT_KAFKA_TOPIC;

		logger.info("Connecting to hosts: " + kafkaHosts);

		kafkaConf = new Properties();
		kafkaConf.put(ProducerConfig.BOOTSTRAP_SERVERS_CONFIG, kafkaHosts);
		kafkaConf.put(ProducerConfig.VALUE_SERIALIZER_CLASS_CONFIG,
				ByteArraySerializer.class.getName());
		kafkaConf.put(ProducerConfig.KEY_SERIALIZER_CLASS_CONFIG,
				ByteArraySerializer.class.getName());
		this.producer = new KafkaProducer<byte[], byte[]>(kafkaConf);
		this.kafkaTopic = kafkaTopic;
		logger.info("Writing to topic " + kafkaTopic);
	}

	/**
	 * write raw bytes into Kafka. An error message will be printed to stderr in
	 * case of a failure
	 */
	@Override
	public synchronized void processRaw(byte[] rawMsg) {
		producer.send(new ProducerRecord<byte[], byte[]>(kafkaTopic, rawMsg),
				new Callback() {

					public void onCompletion(RecordMetadata meta, Exception e) {
						if (e != null) {
							logger.error("Failed to write to kafka", e);
						}
					}
				});

	}

	/**
	 * not implemented
	 */
	@Override
	public synchronized void processDecoded(Sample s) {
		// no action required
	}

	/**
	 * closes KafkaProducer
	 */
	@Override
	public void close() {
		logger.info("Disconnecting Kafka...");
		producer.close();
	}

}
