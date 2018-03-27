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
 * 	Authors:
 * 	    Bertold
 *
 */

#ifndef CONVERTER_H_
#define CONVERTER_H_

#include <stdbool.h>
#include <stdint.h>

typedef struct {
	bool open;
	char* portPath;
	int fd;
} converter;

bool converterTune(converter* driver, uint64_t inputFrequency, uint64_t* ifFrequency, bool* invert);
bool converterInit(converter* driver);

#endif /* CONVERTER_H_ */
