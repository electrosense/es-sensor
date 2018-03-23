//
// Created by rocapal on 6/10/17.
//

#include "RemoveDC.h"


namespace electrosense {


RemoveDC::RemoveDC() {
    mQueueOut = new ReaderWriterQueue<SpectrumSegment*>(100);
}


void RemoveDC::run() {

    std::cout << "[*] RemoveDC block running .... " << std::endl;

    mRunning = true;
    SpectrumSegment* segment;

    if (mQueueIn==NULL || mQueueOut==NULL) {
        throw std::logic_error("Queue[IN|OUT] are NULL!");
    }


    while(mRunning) {


        if (mQueueIn && mQueueIn->try_dequeue(segment)) {

            std::vector<std::complex<float>>& iq_complex = segment->getIQSamples();

            /*
            double real_acc=0, imag_acc=0;

            for (auto c: iq_complex) {
                real_acc += real(c);
                imag_acc += imag(c);
            }

            double mean_real = real_acc / iq_complex.size();
            double mean_imag = imag_acc / iq_complex.size();


            //std::transform(iq_complex.begin(), iq_complex.end(), iq_complex.begin(),
            //               [](std::complex<double> c) -> std::complex<double> { std::complex<double>(real(c)-mean_real,imag(c)-mean_imag); } );

            // Check the performance between std::transform and std::for_each

            std::for_each(iq_complex.begin(), iq_complex.end(), [mean_real, mean_imag](std::complex<double> c) {
                c.real(c.real() - mean_real);
                c.imag(c.imag() - mean_imag);
            } );
            */
/*
            std::cout << "RemoveDC ****************" << std::endl;
            for (unsigned int p = 0; p < iq_complex.size(); p = p + 1) {
                std::complex<float> aux = iq_complex[p];
                std::cout << aux.real() << "," << aux.imag() << ",";
            }
            std::cout << "RemoveDC ****************" << std::endl;
*/
            for (unsigned int p = 0; p < iq_complex.size(); p = p + 1) {
                std::complex<float> aux = iq_complex[p];
                iq_complex[p] = std::complex<float> ( (aux.real() - 127.4)/128.0, (aux.imag() - 127.4)/128.0);
            }

/*
            std::cout << "RemoveDC ****************" << std::endl;
            for (unsigned int p = 0; p < iq_complex.size(); p = p + 1) {
                std::complex<float> aux = iq_complex[p];
                std::cout << aux.real() << "," << aux.imag() << ",";
            }
            std::cout << "RemoveDC ****************" << std::endl;
*/

            mQueueOut->enqueue(segment);

        } else
            usleep(1);

    }

}

int RemoveDC::stop() {

    mRunning = false;
    waitForThread();

    return 1;
}



}