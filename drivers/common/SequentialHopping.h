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
 * 	Authors:
 * 	    Roberto Calvo-Palomino <roberto.calvo@imdea.org>
 *
 */

#ifndef SRC_SEQUENTIALHOPPING_H_
#define SRC_SEQUENTIALHOPPING_H_

#include "../../context/ElectrosenseContext.h"
#include <vector>

class SequentialHopping {
public:
  SequentialHopping();
  virtual ~SequentialHopping();

  uint64_t nextHop();

  bool isRoundFinished();

private:
  uint64_t mFreqStep;
  uint64_t mTotalHops;
  std::vector<uint64_t> mFreqs;

  uint64_t mIndex;
};

#endif /* SRC_SEQUENTIALHOPPING_H_ */
