/*
 * Copyright (C) 2018 by IMDEA Networks Institute
 *
 * This file is part of Electrosense.
 *
 * Electrosense is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Electrosense is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with RTL-Spec.  If not, see <http://www.gnu.org/licenses/>.
 *
 * 	Authors: 	Roberto Calvo-Palomino <roberto.calvo@imdea.org>
 *              Markus Engel
 */

#include "IQStream.h"
#include <cstring>
#define SOCKET_NAME "/tmp/socket"
#define SOCKET_AUDIO "/tmp/socket_audio"

namespace electrosense {

IQStream::IQStream() {

  mSocketEnable = false;
  setDecoderState(false);
}

void IQStream::run() {

  unsigned int num_decoders = 6;
  int id_decoders[num_decoders] = {1, 2, 3, 4, 5, 6};
  char *name_decoders[num_decoders] = {"fm_radio", "am_radio",
                                       "dump1090", "rtl_ais",
                                       "acarsdec", "LTECELL_scanner"};
  bool rational_dec[num_decoders] = {true, true, true, true, true, true};
  float fs_output[num_decoders] = {240e3, 60e3, 2.4e6, 1.6e6, 2.0e6, 1.92e6};
  float ripple[num_decoders] = {-40, -40, -40, -40, -40, -40};
  float transition[num_decoders] = {0.1f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f};
  float cutoff[num_decoders] = {0.4f, 0.25f, 0.2f, 0.2f, 0.2f, 0.2f};
  mRunning = true;

  mSocketEnable = false;
  setDecoderState(true);

  int max_sd, sd;
  std::cout << "[*] IQStream block running .... " << std::endl;

  SpectrumSegment *segment;

  if (mQueueIn == NULL) {
    throw std::logic_error("Queue[IN] is NULL!");
  }

  initSocket();

  int pid = fork();
  int fd;

  if (pid == 0) {

    switch (ElectrosenseContext::getInstance()->getDecoder()) {

    case 1:
      printf("[IQStream] Running decoder: fm_radio\n");

      fd = open(SOCKET_AUDIO, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
      dup2(fd, 1);
      close(fd);

      execl("/usr/share/electrosense/decoders/radio.py", "radio.py", "fm",
            "240000", NULL);

      break;
    case 2:
      printf("[IQStream] Running decoder: am_radio\n");

      fd = open(SOCKET_AUDIO, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
      dup2(fd, 1);
      close(fd);

      execl("/usr/share/electrosense/decoders/radio.py", "radio.py", "am",
            "60000", NULL);

      break;

    case 3:
      printf("[IQStream] Running decoder: ADSB (dump1090)\n");
      execl("/usr/share/electrosense/decoders/dump1090", "dump1090",
            "--esense", SOCKET_NAME, NULL);
      break;

    case 4:
      printf("[IQStream] Running decoder: AIS\n");
      execl("/usr/share/electrosense/decoders/ais_decoder", "ais_decoder",
            NULL);

      break;

    case 5:
      printf("[IQStream] Running decoder: acarsdec\n");
      execl("/usr/share/electrosense/decoders/acarsdec.rtl", "acarsdec.rtl",
            "-e", SOCKET_NAME, NULL);
      break;

    case 6:
      printf("[IQStream] Running decoder: CellSearch\n");
      execl("/usr/share/electrosense/decoders/CellSearch",
            "CellSearch", NULL);
      break;
    }

  } else {

    printf("After the fork!!, the process identifier (pid) "
           "of the parent is still %d\n - fork() returned %d\n",
           (int)getpid(), pid);

    mChildPID = pid;

    // Testing purposes
    Filter::Resampler::ResamplerConfig cfg;

    if (ElectrosenseContext::getInstance()->getDecoder() != 0) {

      cfg.rational =
          rational_dec[ElectrosenseContext::getInstance()->getDecoder() - 1];
      cfg.inputRate = 2.4e6;
      cfg.outputRate =
          fs_output[ElectrosenseContext::getInstance()->getDecoder() - 1];
      cfg.transitionWidthFraction =
          transition[ElectrosenseContext::getInstance()->getDecoder() - 1];
      cfg.desiredRipple =
          ripple[ElectrosenseContext::getInstance()->getDecoder() - 1];
      cfg.cutoffFraction =
          cutoff[ElectrosenseContext::getInstance()->getDecoder() - 1];

    } else {
      // Default values;
      cfg.rational = true;
      cfg.inputRate = 2.4e6;
      cfg.outputRate = 6e3;
      cfg.transitionWidthFraction = 0.2f;
      cfg.desiredRipple = -40;
    }

    Filter::Resampler::Downsampler<uint8_t, uint8_t, uint32_t> ri(cfg);
    Filter::Resampler::Downsampler<uint8_t, uint8_t, uint32_t> rq(cfg);
    std::cout << "Final output rate: " << ri.getActualOutputRate() << std::endl;

    uint8_t *inputVectorI = (uint8_t *)malloc(256 * 1024 * sizeof(uint8_t));
    uint8_t *inputVectorQ = (uint8_t *)malloc(256 * 1024 * sizeof(uint8_t));

    uint8_t *outputVectorI = (uint8_t *)malloc(256 * 1024 * sizeof(uint8_t));
    uint8_t *outputVectorQ = (uint8_t *)malloc(256 * 1024 * sizeof(uint8_t));

    unsigned char *fbuffer =
        (unsigned char *)malloc(256 * 1024 * sizeof(unsigned char));
    while (mRunning) {

      if (!mSocketEnable) {

        fd_set readfds;

        FD_ZERO(&readfds);
        FD_SET(m_fd, &readfds);
        max_sd = m_fd;

        FD_SET(sd, &readfds);

        timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 5000;

        int retval = select(max_sd + 1, &readfds, NULL, NULL, &timeout);

        if (retval) {

          if (FD_ISSET(m_fd, &readfds)) {

            m_cfd = accept(m_fd, (struct sockaddr *)NULL, NULL);
            if (m_cfd < 0)
              error(EXIT_FAILURE, errno, "accept failed");

            printf("[NEW client] Accepted socket\n");
            close(m_fd);

            mSocketEnable = true;
          }
        }
      }

      if (mQueueIn && mQueueIn->try_dequeue(segment)) {

        if (mSocketEnable) {

          if (!mRunning)
            break;

          // ID=3 -> dump1090 does not need to perform resampling

          if (ElectrosenseContext::getInstance()->getDecoder() == 3) {

            ssize_t ret = sendIQSamples(
                m_cfd, ElectrosenseContext::getInstance()->getMinFreq(),
                ElectrosenseContext::getInstance()->getGainDecoder(),
                segment->get_buffer_len(),
                reinterpret_cast<const int8_t *>(segment->get_buffer()));

            if (ret < 0 && mRunning) {
              std::cerr
                  << "[Warning] Error while writing in the socket of the child"
                  << std::endl;

              delete (segment);
              setDecoderState(false);
              std::cout << "Restarting IQStream instance ...." << std::endl;
              break;
            }

          } else {

            unsigned int numSamples = segment->get_buffer_len();
            unsigned char *sbuffer = segment->get_buffer();

            for (unsigned i = 0; i < numSamples; i = i + 2) {
              inputVectorI[i / 2] = sbuffer[i];
              inputVectorQ[i / 2] = sbuffer[i + 1];
            }

            unsigned int outputSamples =
                ri.filter(inputVectorI, numSamples / 2, outputVectorI);
            rq.filter(inputVectorQ, numSamples / 2, outputVectorQ);

            for (unsigned int i = 0; i < outputSamples; i++) {
              fbuffer[2 * i] = outputVectorI[i];
              fbuffer[2 * i + 1] = outputVectorQ[i];
            }

            // if (!mRunning) break;

            ssize_t ret = sendIQSamples(
                m_cfd, ElectrosenseContext::getInstance()->getMinFreq(),
                ElectrosenseContext::getInstance()->getGainDecoder(),
                outputSamples * 2, reinterpret_cast<const int8_t *>(fbuffer));

            if (ret < 0 && mRunning) {
              std::cerr
                  << "[Warning] Error while writing in the socket of the child"
                  << std::endl;

              delete (segment);
              setDecoderState(false);
              std::cout << "Restarting IQStream instance ...." << std::endl;
              break;
            }
          }
        }

        delete (segment);

      } else
        usleep(0.1);
    }

    free(fbuffer);
    free(inputVectorI);
    free(inputVectorQ);
    free(outputVectorI);
    free(outputVectorQ);
  }
}

ssize_t IQStream::sendIQSamples(int fd, uint64_t centerFrequency, uint32_t gain,
                                uint32_t nSamples, const int8_t *iq) {

  struct Header hdr = {.centerFrequency = htole64(centerFrequency),
                       .gain = htole32(gain),
                       .nSamples = htole32(nSamples)};

  struct iovec iov[2] = {{.iov_base = &hdr, .iov_len = 8 + 4 + 4},
                         {.iov_base = (uint8_t *)iq, .iov_len = nSamples}};

  struct msghdr msg;
  std::memset(&msg, 0, sizeof msg);
  msg.msg_iov = iov;
  msg.msg_iovlen = 2;

  return sendmsg(fd, &msg, MSG_NOSIGNAL);
}

void IQStream::initSocket() {

  unlink(SOCKET_NAME);

  m_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (m_fd < 0)
    error(EXIT_FAILURE, errno, "socket failed");
  // printf("Opened socket\n");

  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, SOCKET_NAME, sizeof(addr.sun_path) - 1);
  int rc = bind(m_fd, (struct sockaddr *)&addr, sizeof(addr));
  if (rc < 0)
    error(EXIT_FAILURE, errno, "bind failed");
  // printf("Bound socket\n");

  rc = listen(m_fd, 1);
  if (rc < 0)
    error(EXIT_FAILURE, errno, "listen failed");

  printf("[IQStream] Listening on socket waiting for the DECODER\n");
}

int IQStream::stop() {

  if (mRunning) {
    mRunning = false;

    kill(mChildPID, SIGTERM);
    kill(mChildPID, SIGKILL);
    waitpid(mChildPID, 0, 0);

    close(m_cfd);
    waitForThread();
  }

  return 1;
}
} // namespace electrosense
