/*
 * state.h
 *
 *  Created on: Sep 15, 2010
 *      Author: baj
 */

#ifndef STATE_H_
#define STATE_H_

#include "utils.h"
#include "boost/tuple/tuple.hpp"
#include "boost/tuple/tuple_comparison.hpp"
#include "boost/tuple/tuple_io.hpp"

#include <string>
#include <sstream>

class State: public boost::tuples::tuple<int, int, int, int> {
public:
	State(const int a = 0, const int b = 0, const int c = 0, const int d = 0);

	int x() const;
	int y() const;
	int passenger() const;
	int destination() const ;

	int& x();
	int& y();
	int& passenger();
	int& destination();

	bool absorbing() const;

	std::string str() const;

  size_t hash() const {
      return x() << 24 | y() << 16 | passenger() << 8 | destination();
  }
};


#endif /* STATE_H_ */
