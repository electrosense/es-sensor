/*
 *  Created on: Oct 24, 2018
 *      Author: bertold
 */

#ifndef FILTER_CIC_INTCOMB_H_
#define FILTER_CIC_INTCOMB_H_

namespace Filter {
namespace CIC {

template <typename T> class Comb {
public:
	inline T update(T in){
		T out = in - value_;
		value_ = in;
		return out;
	}
private:
	T value_ = 0;
};

template <typename T> class Integrator {
public:
	inline T update(T in){
		T out = in + value_;
		value_ = out;
		return out;
	}
private:
	T value_ = 0;
};

}
}



#endif /* FILTER_CIC_INTCOMB_H_ */
