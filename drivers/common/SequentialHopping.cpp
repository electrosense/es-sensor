/*
 * Copyright (C) 2018 by IMDEA Networks Institute
 *
 * This file is part of Electrosense.
 *
 * Electrosense is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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
 * 	Authors: 	Roberto Calvo <roberto.calvo@imdea.org>
 *
 */

#include "SequentialHopping.h"

SequentialHopping::SequentialHopping() {

    mIndex = 0;
    mFreqStep = (1 - ElectrosenseContext::getInstance()->getFreqOverlap() )* ElectrosenseContext::getInstance()->getSamplingRate();

    mTotalHops = (ElectrosenseContext::getInstance()->getMaxFreq() - ElectrosenseContext::getInstance()->getMinFreq() + 1e6) / mFreqStep;

    std::cout << ElectrosenseContext::getInstance()->getMinFreq() << " , " <<
                 ElectrosenseContext::getInstance()->getMaxFreq() << std::endl;

    std::cout << mTotalHops << std::endl;

    mFreqs.insert(mFreqs.begin(), ElectrosenseContext::getInstance()->getMinFreq() + 0.5 * mFreqStep);

    for (int i=1; i<(int)mTotalHops; i++)
    {
        mFreqs.insert(mFreqs.end(), mFreqs.at(i-1) + mFreqStep);
    }

}
uint64_t  SequentialHopping::nextHop()
{
    uint64_t ret_freq = mFreqs.at(mIndex);
    mIndex++;

    if (mIndex>=mFreqs.size())
        mIndex = 0;

    return ret_freq;
}

bool SequentialHopping::isRoundFinished()
{
    return mIndex == 0;
}

SequentialHopping::~SequentialHopping() {

}

