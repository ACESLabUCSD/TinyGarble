/*
 This file is part of TinyGarble. It is modified version of JustGarble
 under GNU license.

 TinyGarble is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TinyGarble is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TinyGarble.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SCD_SCHEDULING_H_
#define SCD_SCHEDULING_H_

#include "scd/v_2_scd.h"

int TopSort(const vector<ReadGate>& G, int no_task, int *index);
int Schedule(const ReadCircuit& readCircuit, int no_core, int **core);
void QuickSort(int *arr, int *index, int left, int right);
int GetMinIndex(int *arr, int size);
int GetMax(int *arr, int size);

#endif /* SCD_SCHEDULING_H_ */
