#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include <math.h>
#include <librdkafka/rdkafka.h>

#include "../include/UTI.h"
#include "../include/THR.h"
#include "../include/ITE.h"
#include "../include/QUE.h"
#include "../include/TCP.h"
#include "../include/TLS.h"

#define DEFAULT_THREAD_POOL_SIZE 25
#define DEFAULT_TLS_CREDENTIALS "0"
#define DEFAULT_KAFKA_HOSTS "127.0.0.1:9092"
#define DEFAULT_KAFKA_TOPIC "rtl-spec.frequency-spectrum-v2"

#define THR_COLLECTOR 0
#define THR_RECEPTION 1

#define STR_LEN 30
#define STR_LEN_LONG 512

typedef struct {
  Thread       *thread;
  Queue        *thread_pool;
  unsigned int portnumber;
  unsigned int thread_pool_size;
  char         *tls_credentials;
  char         *kafka_hosts;
  char         *kafka_topic;
} CollectorCTX;

typedef struct {
  Thread         *thread;
  TCP_Connection *tcp_c;
  TLS_Connection *tls_c;
  char           *ca_cert_file;
  char           *cert_file;
  char           *key_file;
  char           *kafka_hosts;
  char           *kafka_topic;
  int            kafka_bandwidth;
} ReceptionCTX;

typedef struct {
  CollectorCTX *coll_ctx;
  ReceptionCTX *recp_ctx;
} ReceptionARG;

typedef struct {
  Thread *thread;
} DecompressionCTX;

typedef struct {
  ReceptionCTX     *recp_ctx;
  DecompressionCTX *decmpr_ctx;
  Queue            *qin, **qsout;
  int              qsout_cnt;
} DecompressionARG;

typedef struct {
  Thread *thread;
  char   *kafka_hosts;
  char   *kafka_topic;
  int    kafka_bandwidth;
} KafkaTransmissionCTX;

typedef struct {
  ReceptionCTX         *recp_ctx;
  KafkaTransmissionCTX *kafka_trns_ctx;
  Queue                *qin, **qsout;
  int                  qsout_cnt;
} KafkaTransmissionARG;

static void* reception(void *args);

static void* kafka_transmission(void *args);

int main(int argc, char *argv[]) {
  int            i;
  char           *ca_cert_file = NULL, *cert_file = NULL, *key_file = NULL;
  char           kafka_host_lst[STR_LEN_LONG], *kafka_hosts, *bandwidth;
  int            kafka_bandwidth;
  
  CollectorCTX   *coll_ctx;
  ReceptionCTX   **recp_ctx = NULL, *r_ctx;
  ReceptionARG   *recp_arg;
  
  TCP_Connection *tcp_wel = NULL, *tcp_cli = NULL;
  
  // Ctrl-C signal catcher
  void terminate(int sig) {
    
    if(sig == SIGINT) fprintf(stderr, "\nCtrl-C caught. Waiting for termination...\n");
    
    // Release TCP and SSL/TLS connections
#if defined(VERBOSE)
    tcp_disconnect_p(tcp_wel);
    tcp_release_p(tcp_wel);
    free(tcp_cli);
#else
    tcp_disconnect(tcp_wel);
    tcp_release(tcp_wel);
    free(tcp_cli);
#endif
    
    // Ask reception threads to terminate
    for(i=0; i<coll_ctx->thread_pool_size; ++i) {
      pthread_mutex_lock(recp_ctx[i]->thread->lock);
      recp_ctx[i]->thread->is_running = 0;
      pthread_mutex_unlock(recp_ctx[i]->thread->lock);
      pthread_cond_signal(recp_ctx[i]->thread->awake);
    }
    
    // Terminate and release reception threads
    for(i=0; i<coll_ctx->thread_pool_size; ++i) {
      // Join reception threads
      pthread_join(*(recp_ctx[i]->thread->fd), NULL);
      // Release reception threads
      THR_release(recp_ctx[i]->thread);
      // Release reception context
      free(recp_ctx[i]);
    }
    free(recp_ctx);
    
    // Release collector context
    THR_release(coll_ctx->thread);
    QUE_release(coll_ctx->thread_pool);
    free(coll_ctx);
    
    fprintf(stderr, "Terminated.\n");
    pthread_exit(NULL);
  }

  // Parse arguments and options
  void parse_args(int argc, char *argv[]) {
    int opt;
    const char *options = "hp:s:m:u:";
    
    // Option arguments
    while((opt = getopt(argc, argv, options)) != -1) {
      switch(opt) {
	case 'h':
	  goto usage;
	case 'p':
	  coll_ctx->thread_pool_size = atol(optarg);
	  break;
	case 's':
	  coll_ctx->tls_credentials = optarg;
	  break;
	case 'm':
	  coll_ctx->kafka_hosts = optarg;
	  break;
	case 'u':
	  coll_ctx->kafka_topic = optarg;
	  break;
	default:
	  goto usage;
      }
    }
    
    // Non-option arguments
    if(optind+1 != argc) {
      usage:
      fprintf(stderr,
	"Usage:\n"
	"  %s portnumber\n"
	"  [-h]\n"
	"  [-p <thread_pool_size>]\n"
	"  [-s <ca_cert>:<cert>:<key>]\n"
	"  [-m <hostname1>:<portnumber1>,...,<hostnameN>:<portnumberN>[;<bandwidth>]]\n"
	"  [-u <topic>]\n"
	"\n"
	"Arguments:\n"
	"  portnumber               Port number\n"
	"\n"
	"Options:\n"
	"  -h                       Show this help\n"
	"  -p <thread_pool_size>    Maximal number of simultaneous connections [default=%u]\n"
	"  -s <ca_cert>:<cert>:<key>\n"
	"                           SSL/TLS (CA) certificate and private key files [default=%s]\n"
	"                             0 for TCP instead SSL/TLS collector\n"
	"  -m <hostname1>:<portnumber1>,...,<hostnameN>:<portnumberN>[;<bandwidth>]\n"
	"                           Kafka brokers [default=%s]\n"
	"                             Bandwidth limitation in Kb/s\n"
	"  -u <topic>               Kafka topic [default=%s]\n"
	"",
	argv[0],
	coll_ctx->thread_pool_size,
	coll_ctx->tls_credentials,
	coll_ctx->kafka_hosts,
	coll_ctx->kafka_topic);
      exit(1);
    } else {
      coll_ctx->portnumber = atol(argv[optind]);
    }
  }
  
  // Initialize collector context
  coll_ctx = (CollectorCTX *) malloc(sizeof(CollectorCTX));
  coll_ctx->thread_pool_size = DEFAULT_THREAD_POOL_SIZE;
  coll_ctx->tls_credentials = DEFAULT_TLS_CREDENTIALS;
  coll_ctx->kafka_hosts = DEFAULT_KAFKA_HOSTS;
  coll_ctx->kafka_topic = DEFAULT_KAFKA_TOPIC;
  parse_args(argc, argv);
  coll_ctx->thread_pool = QUE_initialize(coll_ctx->thread_pool_size);
  THR_initialize(&(coll_ctx->thread), THR_COLLECTOR);
  
  // Parse SSL/TLS credentials
  if(strcmp(coll_ctx->tls_credentials, DEFAULT_TLS_CREDENTIALS) != 0) {
    ca_cert_file = strtok(coll_ctx->tls_credentials, ":");
    if(ca_cert_file == NULL) {
      fprintf(stderr, "[COLL] ERROR: Invalid SSL/TLS CA certificate file.\n");
      exit(1);
    }
    cert_file = strtok(NULL, ":");
    if(cert_file == NULL) {
      fprintf(stderr, "[COLL] ERROR: Invalid SSL/TLS certificate file.\n");
      exit(1);
    }
    key_file = strtok(NULL, ":");
    if(key_file == NULL) {
      fprintf(stderr, "[COLL] ERROR: Invalid SSL/TLS private key file.\n");
      exit(1);
    }
  }

  // Parse Kafka hosts
  strncpy(kafka_host_lst, coll_ctx->kafka_hosts, STR_LEN_LONG);
  kafka_hosts = strtok(kafka_host_lst, ";");
  bandwidth = strtok(NULL, ";");
  if(bandwidth != NULL) kafka_bandwidth = atoi(bandwidth);
  else kafka_bandwidth = 0;
    
  // Initialize reception threads
  recp_ctx = (ReceptionCTX **) malloc(coll_ctx->thread_pool_size*sizeof(ReceptionCTX *));
  for(i=0; i<coll_ctx->thread_pool_size; ++i) {
    // Initialize reception context
    recp_ctx[i] = (ReceptionCTX *) malloc(sizeof(ReceptionCTX));
    recp_ctx[i]->tcp_c = NULL;
    recp_ctx[i]->tls_c = NULL;
    recp_ctx[i]->ca_cert_file = ca_cert_file;
    recp_ctx[i]->cert_file = cert_file;
    recp_ctx[i]->key_file = key_file;
    recp_ctx[i]->kafka_hosts = kafka_hosts;
    recp_ctx[i]->kafka_bandwidth = kafka_bandwidth;
    recp_ctx[i]->kafka_topic = coll_ctx->kafka_topic;
    THR_initialize(&(recp_ctx[i]->thread), THR_RECEPTION+i);
    // Initialize reception arguments
    recp_arg = (ReceptionARG *) malloc(sizeof(ReceptionARG));
    recp_arg->coll_ctx = coll_ctx;
    recp_arg->recp_ctx = recp_ctx[i];
    // Start reception threads
    pthread_mutex_lock(recp_ctx[i]->thread->lock);
    pthread_create(recp_ctx[i]->thread->fd, NULL, reception, recp_arg);
    pthread_cond_wait(coll_ctx->thread->awake, recp_ctx[i]->thread->lock);
    pthread_mutex_unlock(recp_ctx[i]->thread->lock);
    // Add reception thread to thread pool
    QUE_insert(coll_ctx->thread_pool, recp_ctx[i]);
  }
  
#if defined(VERBOSE)
  fprintf(stderr, "[COLL] Started.\n");
#endif

    // Initialize TCP connections
#if defined(VERBOSE)
  tcp_init_p(&tcp_wel, NULL, coll_ctx->portnumber);
  tcp_listen_p(tcp_wel, coll_ctx->thread_pool_size);
#else
  tcp_init(&tcp_wel, NULL, coll_ctx->portnumber);
  tcp_listen(tcp_wel, coll_ctx->thread_pool_size);
#endif
  
  // Ctrl-C signal catcher
  signal(SIGINT, terminate);
  
  while(1) {
    // Wait for thread pool not being empty
    pthread_mutex_lock(coll_ctx->thread_pool->mut);
    while(coll_ctx->thread_pool->empty) {
      pthread_cond_wait(coll_ctx->thread_pool->notEmpty, coll_ctx->thread_pool->mut);
    }
    // Pull thread out of the pool
    r_ctx = (ReceptionCTX *) QUE_remove(coll_ctx->thread_pool);
    pthread_mutex_unlock(coll_ctx->thread_pool->mut);
    pthread_cond_signal(coll_ctx->thread_pool->notFull);
    
    // Accept incoming TCP connections
#if defined(VERBOSE)
    tcp_accept_p(tcp_wel, &tcp_cli);
#else
    tcp_accept(tcp_wel, &tcp_cli);
#endif
    
    // Awake thread to serve TCP connection
    pthread_mutex_lock(r_ctx->thread->lock);
    r_ctx->tcp_c = tcp_cli;
    pthread_cond_signal(r_ctx->thread->awake);
    pthread_mutex_unlock(r_ctx->thread->lock);
  }
  
  return 0;
}

static void* reception(void *args) {
  int                  q_size;
  uint32_t             reduced_fft_size, data_size, payload_size, *payload_buf;
  char                 *ca_cert_file, *cert_file, *key_file;
  
  ReceptionARG         *recp_arg;
  CollectorCTX         *coll_ctx;
  ReceptionCTX         *recp_ctx;
  
  TCP_Connection       *tcp_c = NULL;
  TLS_Connection       *tls_c = NULL;
  
  Item                 *iout;
  
  Queue                *q_kafka_trns = NULL;
  KafkaTransmissionCTX *kafka_trns_ctx;
  KafkaTransmissionARG *kafka_trns_arg;
  
  // Parse arguments
  recp_arg = (ReceptionARG *) args;
  coll_ctx = (CollectorCTX *) recp_arg->coll_ctx;
  recp_ctx = (ReceptionCTX *) recp_arg->recp_ctx;
  
  ca_cert_file = recp_ctx->ca_cert_file;
  cert_file = recp_ctx->cert_file;
  key_file = recp_ctx->key_file;
  
#if defined(VERBOSE) || defined(VERBOSE_RECP)
  fprintf(stderr, "[RECP] ID:\t%u\t Started.\n", recp_ctx->thread->id);
#endif
  
  // Signal collector that we are ready
  pthread_mutex_lock(recp_ctx->thread->lock);
  pthread_cond_signal(coll_ctx->thread->awake);
  while(recp_ctx->thread->is_running) {
    // Await requests
    pthread_cond_wait(recp_ctx->thread->awake, recp_ctx->thread->lock);
    if(!recp_ctx->thread->is_running) break;

    // BEGIN SERVING REQUEST
#if defined(VERBOSE) || defined(VERBOSE_RECP)
    fprintf(stderr, "[RECP] ID:\t%u\t Request received.\n", recp_ctx->thread->id);
#endif

    tcp_c = recp_ctx->tcp_c;
    
    // Initialize data processing queues
    q_size = 1000;
    q_kafka_trns = QUE_initialize(q_size);
    
    // Initialize data processing contexts
    kafka_trns_ctx = (KafkaTransmissionCTX *) malloc(sizeof(KafkaTransmissionCTX));
    kafka_trns_ctx->kafka_hosts = recp_ctx->kafka_hosts;
    kafka_trns_ctx->kafka_bandwidth = recp_ctx->kafka_bandwidth;
    kafka_trns_ctx->kafka_topic = recp_ctx->kafka_topic;
    THR_initialize(&(kafka_trns_ctx->thread), recp_ctx->thread->id);

    // Initialize data processing arguments
    kafka_trns_arg = (KafkaTransmissionARG *) malloc(sizeof(KafkaTransmissionARG));
    kafka_trns_arg->recp_ctx = recp_ctx;
    kafka_trns_arg->kafka_trns_ctx = kafka_trns_ctx;
    kafka_trns_arg->qin = q_kafka_trns;
    kafka_trns_arg->qsout_cnt = 0;
    kafka_trns_arg->qsout = (Queue **) malloc(kafka_trns_arg->qsout_cnt*sizeof(Queue *));
    
    // Start data processing threads
    pthread_create(kafka_trns_ctx->thread->fd, NULL, kafka_transmission, kafka_trns_arg);

    // TLS reception
    if(key_file != NULL) {
    // Initialize and accept SSL/TLS connection
#if defined(VERBOSE) || defined(VERBOSE_RECP)
      tls_init_p(&tls_c, tcp_c, TLSv1_1_server_method(), ca_cert_file, cert_file, key_file, NULL, 0);
      tls_accept_p(tls_c);
#else
      tls_init(&tls_c, tcp_c, TLSv1_1_server_method(), ca_cert_file, cert_file, key_file, NULL, 0);
      tls_accept(tls_c);
#endif
    }
    
    // Serve request
    while(recp_ctx->thread->is_running) {
      pthread_mutex_unlock(recp_ctx->thread->lock);
      
      // Read header
      if(key_file == NULL) {
#if defined(VERBOSE) || defined(VERBOSE_RECP)
	tcp_read_p(tcp_c, &data_size, sizeof(uint32_t));
#else
	tcp_read(tcp_c, &data_size, sizeof(uint32_t));
#endif
      } else {
#if defined(VERBOSE) || defined(VERBOSE_RECP)
	tls_read_p(tls_c, &data_size, sizeof(uint32_t));
#else
	tls_read(tls_c, &data_size, sizeof(uint32_t));
#endif
      }
      data_size = ntohl(data_size);
      if(data_size <= 0) break;

      if(key_file == NULL) {
#if defined(VERBOSE) || defined(VERBOSE_RECP)
	tcp_read_p(tcp_c, &reduced_fft_size, sizeof(uint32_t));
#else
	tcp_read(tcp_c, &reduced_fft_size, sizeof(uint32_t));
#endif
      } else {
#if defined(VERBOSE) || defined(VERBOSE_RECP)
	tls_read_p(tls_c, &reduced_fft_size, sizeof(uint32_t));
#else
	tls_read(tls_c, &reduced_fft_size, sizeof(uint32_t));
#endif
      }
      reduced_fft_size = ntohl(reduced_fft_size);
      if(reduced_fft_size <= 0) break;
      
      // Read payload
      payload_size = (data_size + 3) & ~0x03;
      payload_buf = (uint32_t *) malloc(payload_size);

      if(key_file == NULL) {
#if defined(VERBOSE) || defined(VERBOSE_RECP)
	tcp_read_p(tcp_c, payload_buf, payload_size);
#else
	tcp_read(tcp_c, payload_buf, payload_size);
#endif
      } else {
#if defined(VERBOSE) || defined(VERBOSE_RECP)
	tls_read_p(tls_c, payload_buf, payload_size);
#else
	tls_read(tls_c, payload_buf, payload_size);
#endif
      }
      
      // Initialize output item
      iout = ITE_init();
      iout->reduced_fft_size = reduced_fft_size;
      iout->data_size = data_size;
      iout->data = payload_buf;
      
      // Wait for output queue not being full
      pthread_mutex_lock(q_kafka_trns->mut);
      while(q_kafka_trns->full) {
	pthread_cond_wait(q_kafka_trns->notFull, q_kafka_trns->mut);
#if defined(VERBOSE) || defined(VERBOSE_RECP)
	fprintf(stderr, "[RECP] ID:\t%u\t QA full.\n", recp_ctx->thread->id);
#endif
      }
      // Write item to output queue
      QUE_insert(q_kafka_trns, iout);
      pthread_mutex_unlock(q_kafka_trns->mut);
      pthread_cond_signal(q_kafka_trns->notEmpty);
#if defined(VERBOSE) || defined(VERBOSE_RECP)
      fprintf(stderr, "[RECP] ID:\t%u\t Push item.\n", recp_ctx->thread->id);
#endif
      
      pthread_mutex_lock(recp_ctx->thread->lock);
    }
    
    // Signal that we are done and no further items will appear in the queue
    pthread_mutex_lock(q_kafka_trns->mut);
    q_kafka_trns->exit = 1;
    pthread_cond_signal(q_kafka_trns->notEmpty);
    pthread_cond_signal(q_kafka_trns->notFull);
    pthread_mutex_unlock(q_kafka_trns->mut);
    
    // Join data processing threads
    pthread_join(*(kafka_trns_ctx->thread->fd), NULL);
    
    // Free output queues
    free(kafka_trns_arg->qsout);
    
    // Free data processing arguments
    free(kafka_trns_arg);
    
    // Free data processing contexts
    THR_release(kafka_trns_ctx->thread);
    free(kafka_trns_ctx);
    
    // Free data processing queues
    QUE_release(q_kafka_trns);

    // TCP reception
    if(key_file == NULL) {
      // Disconnect TCP connection
#if defined(VERBOSE) || defined(VERBOSE_RECP)
      tcp_disconnect_p(tcp_c);
      tcp_release_p(tcp_c);
#else
      tcp_disconnect(tcp_c);
      tcp_release(tcp_c);
#endif
    }
    // TLS reception
    else {
      // Disconnect SSL/TLS connection
#if defined(VERBOSE) || defined(VERBOSE_RECP)
      tls_disconnect_p(tls_c);
      tls_release_p(tls_c);
#else
      tls_disconnect(tls_c);
      tls_release(tls_c);
#endif
    }
    
#if defined(VERBOSE) || defined(VERBOSE_RECP)
    fprintf(stderr, "[RECP] ID:\t%u\t Request served.\n", recp_ctx->thread->id);
#endif
    recp_ctx->tls_c = NULL;
    
    // END SERVING REQUEST

    // Wait for thread pool not being full
    pthread_mutex_lock(coll_ctx->thread_pool->mut);
    while(coll_ctx->thread_pool->full) {
#if defined(VERBOSE) || defined(VERBOSE_RECP)
      fprintf(stderr, "[RECP] ID:\t%u\t Thread poll full.\n", recp_ctx->thread->id);
#endif
      pthread_cond_wait(coll_ctx->thread_pool->notFull, coll_ctx->thread_pool->mut);
    }
    // Reinsert thread to pool
    QUE_insert(coll_ctx->thread_pool, recp_ctx);
    pthread_mutex_unlock(coll_ctx->thread_pool->mut);
    pthread_cond_signal(coll_ctx->thread_pool->notEmpty);    
  }
  
#if defined(VERBOSE) || defined(VERBOSE_RECP)
    tcp_disconnect_p(recp_ctx->tcp_c);
    tcp_release_p(recp_ctx->tcp_c);
#else
    tcp_disconnect(recp_ctx->tcp_c);
    tcp_release(recp_ctx->tcp_c);
#endif

  pthread_mutex_unlock(recp_ctx->thread->lock);
  
#if defined(VERBOSE) || defined(VERBOSE_RECP)
  fprintf(stderr, "[RECP] ID:\t%u\t Terminated.\n", recp_ctx->thread->id);
#endif
  
  free(recp_arg);
  
  pthread_exit(NULL);
}

static void* kafka_transmission(void *args) {

  char                    *kafka_hosts;
  char                    *kafka_topic;
  char                    kafka_errstr[STR_LEN];
  
  rd_kafka_conf_t         *kafka_conf;
  rd_kafka_topic_conf_t   *kafka_topic_conf;
  rd_kafka_t              *kafka_rk = NULL;
  rd_kafka_topic_t        *kafka_rkt = NULL;
  
  int                     kafka_bandwidth;
  int                     bytes_sent = 0;
  UTI_BandwidthController *bc = NULL;
  
  Queue                   *qin;
  Item                    *iin;
  
  KafkaTransmissionARG    *kafka_trns_arg;
  KafkaTransmissionCTX    *kafka_trns_ctx;
  
  // Parse arguments
  kafka_trns_arg = (KafkaTransmissionARG *) args;
  kafka_trns_ctx = (KafkaTransmissionCTX *) kafka_trns_arg->kafka_trns_ctx;
  qin = (Queue *) kafka_trns_arg->qin;
  kafka_hosts = kafka_trns_ctx->kafka_hosts;
  kafka_topic = kafka_trns_ctx->kafka_topic;
  kafka_bandwidth = kafka_trns_ctx->kafka_bandwidth;
  
#if defined(RPI_GPU) && (defined(VERBOSE) || defined(VERBOSE_KAFKA_TRNS) || defined(TID))
  fprintf(stderr, "[KTRS] Started.\tTID: %li\n", (long int) syscall(224));
#endif
  
  // Apache Kafka initialization

  void msg_delivered(rd_kafka_t *rk,
		     void *payload, size_t len,
		     int error_code,
		     void *opaque, void *msg_opaque) {
    if(error_code)
      fprintf(stderr, "[KTRS] ERROR: Message delivery failed: %s\n", rd_kafka_err2str(error_code));
  }

  // Create Kafka configuration
  kafka_conf = rd_kafka_conf_new();
  // Create Kafka topic configuration
  kafka_topic_conf = rd_kafka_topic_conf_new();
  // Set up message delivery report callback
  rd_kafka_conf_set_dr_cb(kafka_conf, msg_delivered);
  // Create Kafka producer
  if(!(kafka_rk = rd_kafka_new(RD_KAFKA_PRODUCER, kafka_conf, kafka_errstr, sizeof(kafka_errstr)))) {
    fprintf(stderr, "[KTRS] ERROR: Failed to create new Kafka producer: %s\n", kafka_errstr);
    exit(1);
  }
  // Add Kafka brokers
  if(rd_kafka_brokers_add(kafka_rk, kafka_hosts) == 0) {
    fprintf(stderr, "[KTRS] ERROR: No valid brokers specified.\n");
    exit(1);
  }
  // Create Kafka topic
  kafka_rkt = rd_kafka_topic_new(kafka_rk, kafka_topic, kafka_topic_conf);
  
  // Initialize bandwidth controller
  bc = UTI_initialize_bandwidth_controller(kafka_bandwidth);

  while(1) {
    // Wait for input queue not being empty
    pthread_mutex_lock(qin->mut);
    while(qin->empty) {
      // No more input is coming to this queue
      if(qin->exit) {
	pthread_mutex_unlock(qin->mut);
	goto EXIT;
      }
      // Wait for more input coming to this queue
      pthread_cond_wait(qin->notEmpty, qin->mut);
    }
   
    // Read item from input queue
    iin = (Item *) QUE_remove(qin);
    pthread_mutex_unlock(qin->mut);
    pthread_cond_signal(qin->notFull);
    
#if defined(VERBOSE) || defined(VERBOSE_KAFKA_TRNS)
    fprintf(stderr, "[KTRS] Pull item.\n");
#endif

    // Enforce bandwidth throttling
    UTI_enforce_bandwidth_throttling(bc, bytes_sent);
    
    // Send/produce message
    if(rd_kafka_produce(kafka_rkt, RD_KAFKA_PARTITION_UA, RD_KAFKA_MSG_F_COPY,
      iin->data, iin->data_size, NULL, 0, NULL) == -1) {
      fprintf(stderr, "[TTRS] ERROR: Failed producing to topic %s partition %i: %s\n",
	      rd_kafka_topic_name(kafka_rkt), RD_KAFKA_PARTITION_UA,
	      rd_kafka_err2str(rd_kafka_errno2err(errno)));
    }
    // Poll to handle delivery report
    rd_kafka_poll(kafka_rk, 0);
    
    // Bytes sent
    bytes_sent = iin->data_size;

    // Release item
    ITE_free(iin);
    
#if defined(VERBOSE) || defined(VERBOSE_KAFKA_TRNS)
    fprintf(stderr, "[KTRS] Push item.\n");
#endif

  }
  
  EXIT:
  
  // Release bandwidth controller
  UTI_release_bandwidth_controller(bc);

  // Wait for messages to be delivered
  while(rd_kafka_outq_len(kafka_rk) > 0) rd_kafka_poll(kafka_rk, 100);
  // Destroy Kafka topic
  rd_kafka_topic_destroy(kafka_rkt);
  // Destroy Kafka producer
  rd_kafka_destroy(kafka_rk);
  
#if defined(VERBOSE) || defined(VERBOSE_KAFKA_TRNS)
  fprintf(stderr, "[KTRS] Terminated.\n");
#endif
  
  pthread_exit(NULL);
}

