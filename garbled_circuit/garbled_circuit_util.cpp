/*
 This file is part of JustGarble.

 JustGarble is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 JustGarble is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with JustGarble.  If not, see <http://www.gnu.org/licenses/>.

 */
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

#include "garbled_circuit/garbled_circuit_util.h"

#include "util/common.h"
#include "util/log.h"
#include "util/util.h"

using std::endl;

uint64_t NumOfNonXor(const GarbledCircuit& garbled_circui) {
  uint64_t num_of_non_xor = 0;
  for (uint64_t i = 0; i < garbled_circui.gate_size; i++) {
    if (garbled_circui.garbledGates[i].type != XORGATE
        && garbled_circui.garbledGates[i].type != XNORGATE
        && garbled_circui.garbledGates[i].type != NOTGATE) {
      num_of_non_xor++;
    }
  }
  return num_of_non_xor;
}

void RemoveGarbledCircuit(GarbledCircuit *garbled_circuit) {
  delete[] garbled_circuit->garbledGates;
  if (garbled_circuit->outputs != nullptr) {
    delete[] garbled_circuit->outputs;
  }
  if (garbled_circuit->I != nullptr) {
    delete[] garbled_circuit->I;
  }
  if (garbled_circuit->D != nullptr) {
    delete[] garbled_circuit->D;
  }
}

int ParseInitInputStr(const string& init_str, const string&input_str,
                      uint64_t init_size, uint64_t input_size,
                      uint64_t clock_cycles, BIGNUM** init, BIGNUM** input) {
  BN_hex2bn(init, init_str.c_str());
  BN_hex2bn(input, input_str.c_str());

  return SUCCESS;
}

inline void HalfGarbleEvalGateKnownValue(int type, int knwon_wire_ind,
                                         short input_value,
                                         short* output_value) {
  bool x0, x1;
  if (knwon_wire_ind == 0) {
    x0 = GateOperator(type, input_value, 0);
    x1 = GateOperator(type, input_value, 1);
  } else {
    x0 = GateOperator(type, 0, input_value);
    x1 = GateOperator(type, 1, input_value);
  }

  if (x0 == x1) {
    if (x0 == 0) {
      *output_value = 0;
    } else {
      *output_value = 1;
    }
  } else {
    *output_value = UNKOWN;
  }
}

inline void HalfGarbleGate(int type, int knwon_wire_ind, short input_value,
                           BlockPair input_labels, BlockPair* output_labels,
                           short* output_value) {
  bool x0, x1;
  if (knwon_wire_ind == 0) {
    x0 = GateOperator(type, input_value, 0);
    x1 = GateOperator(type, input_value, 1);
  } else {
    x0 = GateOperator(type, 0, input_value);
    x1 = GateOperator(type, 1, input_value);
  }

  if (x0 == x1) {
    if (x0 == 0) {
      *output_value = 0;
    } else {
      *output_value = 1;
    }
  } else if (x0 == 0 && x1 == 1) {
    *output_labels = input_labels;
    *output_value = UNKOWN;
  } else /*if(x0 == 1 && x1 == 0)*/{
    output_labels->label0 = input_labels.label1;
    output_labels->label1 = input_labels.label0;
    *output_value = UNKOWN;
  }
}

void GarbleEvalGateKnownValue(short input0_value, short input1_value, int type,
                              short* output_value) {
  if (input0_value != UNKOWN && input1_value != UNKOWN) {
    *output_value = GateOperator(type, input0_value, input1_value);
  } else if (input0_value != UNKOWN) {
    HalfGarbleEvalGateKnownValue(type, 0, input0_value, output_value);

  } else if (input1_value != UNKOWN) {
    HalfGarbleEvalGateKnownValue(type, 1, input1_value, output_value);
  } else {
    *output_value = UNKOWN;
  }
}

void GarbleGate(BlockPair input0_labels, short input0_value,
                BlockPair input1_labels, short input1_value, int type,
                uint64_t cid, uint64_t gid, block* garbled_tables,
                uint64_t* garbled_table_ind, block R, AES_KEY AES_Key,
                BlockPair* output_labels, short* output_value) {

  if (input0_value != UNKOWN && input1_value != UNKOWN) {
    *output_value = GateOperator(type, input0_value, input1_value);
  } else if (input0_value != UNKOWN) {
    HalfGarbleGate(type, 0, input0_value, input1_labels, output_labels,
                   output_value);

  } else if (input1_value != UNKOWN) {
    HalfGarbleGate(type, 1, input1_value, input0_labels, output_labels,
                   output_value);
  } else {
    *output_value = UNKOWN;

    if (type == XORGATE) {
      output_labels->label0 = XorBlock(input0_labels.label0,
                                       input1_labels.label0);
      output_labels->label1 = XorBlock(input0_labels.label1,
                                       input1_labels.label0);
    } else if (type == XNORGATE) {
      output_labels->label0 = XorBlock(input0_labels.label1,
                                       input1_labels.label0);
      output_labels->label1 = XorBlock(input0_labels.label0,
                                       input1_labels.label0);
    } else if (type == NOTGATE) {
      output_labels->label0 = input0_labels.label1;
      output_labels->label1 = input0_labels.label0;
    } else {

      block A1;
      block A0;
      unsigned short v = Type2V(type);
      if (v & 1) {
        A1 = (input0_labels.label0);
        A0 = (input0_labels.label1);
      } else {
        A0 = (input0_labels.label0);
        A1 = (input0_labels.label1);
      }

      block B1;
      block B0;
      if (v & 2) {
        B1 = (input1_labels.label0);
        B0 = (input1_labels.label1);
      } else {
        B0 = (input1_labels.label0);
        B1 = (input1_labels.label1);
      }

      unsigned short pa = get_LSB(A0);
      unsigned short pb = get_LSB(B0);

      block tweak0 = MakeBlock(cid, 2 * gid + 0);
      block tweak1 = MakeBlock(cid, 2 * gid + 1);

      block keys[4];
      keys[0] = XorBlock(A0, tweak0);
      keys[1] = XorBlock(A1, tweak0);
      keys[2] = XorBlock(B0, tweak1);
      keys[3] = XorBlock(B1, tweak1);

      block mask[4];
      mask[0] = keys[0];
      mask[1] = keys[1];
      mask[2] = keys[2];
      mask[3] = keys[3];
      AESEcbEncryptBlks(keys, 4, &(AES_Key));

      mask[0] = XorBlock(mask[0], keys[0]);
      mask[1] = XorBlock(mask[1], keys[1]);
      mask[2] = XorBlock(mask[2], keys[2]);
      mask[3] = XorBlock(mask[3], keys[3]);

      block table[2];
      table[0] = XorBlock(mask[0], mask[1]);
      if (pb) {
        table[0] = XorBlock(table[0], R);
      }
      block G = mask[0];
      if (pa) {
        G = XorBlock(G, table[0]);
      }
      table[1] = XorBlock(mask[2], mask[3]);
      table[1] = XorBlock(table[1], A0);

      block E = mask[2];
      if (pb) {
        E = XorBlock(E, table[1]);
        E = XorBlock(E, A0);
      }

      block C0;
      block C1;
      if (v & 4) {
        C1 = XorBlock(G, E);
        C0 = XorBlock(R, C1);
      } else {
        C0 = XorBlock(G, E);
        C1 = XorBlock(R, C0);
      }

      output_labels->label0 = C0;
      output_labels->label1 = C1;

      for (uint64_t j = 0; j < 2; j++) {
        garbled_tables[(*garbled_table_ind)++] = table[j];
        DUMP("table") << table[j] << endl;
      }
    }
  }
}

inline void HalfEvalGate(int type, int knwon_wire_ind, short input_value,
                         block input_labels, block* output_labels,
                         short* output_value) {

  bool x0, x1;
  if (knwon_wire_ind == 0) {
    x0 = GateOperator(type, input_value, 0);
    x1 = GateOperator(type, input_value, 1);
  } else {
    x0 = GateOperator(type, 0, input_value);
    x1 = GateOperator(type, 1, input_value);
  }

  if (x0 == x1) {
    if (x0 == 0) {
      *output_value = 0;
    } else {
      *output_value = 1;
    }
  } else {
    *output_labels = input_labels;
    *output_value = UNKOWN;
  }
}

void EvalGate(block input0_labels, short input0_value, block input1_labels,
              short input1_value, int type, uint64_t cid, uint64_t gid,
              block* garbled_tables, uint64_t* garbled_table_ind,
              AES_KEY AES_Key, block* output_labels, short* output_value) {
  if (input0_value != UNKOWN && input1_value != UNKOWN) {
    *output_value = GateOperator(type, input0_value, input1_value);
  } else if (input0_value != UNKOWN) {
    HalfEvalGate(type, 0, input0_value, input1_labels, output_labels,
                 output_value);
  } else if (input1_value != UNKOWN) {
    HalfEvalGate(type, 1, input1_value, input0_labels, output_labels,
                 output_value);
  } else {
    *output_value = UNKOWN;

    if (type == XORGATE || type == XNORGATE) {
      *output_labels = XorBlock(input0_labels, input1_labels);
    } else if (type == NOTGATE) {
      *output_labels = input0_labels;
    } else {
      block A = (input0_labels);
      block B = (input1_labels);

      unsigned short sa = get_LSB(A);
      unsigned short sb = get_LSB(B);

      block tweak0 = MakeBlock(cid, 2 * gid + 0);
      block tweak1 = MakeBlock(cid, 2 * gid + 1);

      block table[2];
      for (uint64_t j = 0; j < 2; j++) {
        table[j] = garbled_tables[(*garbled_table_ind)++];
        //CHECK(RecvData(connfd, &(table[j]), sizeof(block)));
        DUMP("table") << table[j] << endl;
      }

      block keys[2];
      keys[0] = XorBlock(A, tweak0);
      keys[1] = XorBlock(B, tweak1);

      block mask[2];
      mask[0] = keys[0];
      mask[1] = keys[1];
      AESEcbEncryptBlks(keys, 2, &(AES_Key));

      mask[0] = XorBlock(mask[0], keys[0]);
      mask[1] = XorBlock(mask[1], keys[1]);

      block G = mask[0];
      if (sa) {
        G = XorBlock(G, table[0]);
      }

      block E = mask[1];
      if (sb) {
        E = XorBlock(E, table[1]);
        E = XorBlock(E, A);
      }

      block C = XorBlock(E, G);

      *output_labels = C;
    }
  }
}

int FillFanout(GarbledCircuit* garbled_circuit) {

  int64_t gate_bias = (int64_t) garbled_circuit->get_gate_lo_index();
  for (uint64_t gid = 0; gid < garbled_circuit->gate_size; gid++) {
    garbled_circuit->garbledGates[gid].fanout = 0;

    GarbledGate garbledGate = garbled_circuit->garbledGates[gid];
    int64_t input0 = garbledGate.input0;
    int64_t input1 = garbledGate.input1;

    if (input0 >= gate_bias) {
      garbled_circuit->garbledGates[input0 - gate_bias].fanout++;
    }
    if (input1 >= gate_bias) {
      garbled_circuit->garbledGates[input1 - gate_bias].fanout++;
    }
  }

  for (uint64_t did = 0; did < garbled_circuit->dff_size; did++) {
    if (garbled_circuit->D[did] >= gate_bias) {
      int64_t gid = garbled_circuit->D[did] - gate_bias;
      garbled_circuit->garbledGates[gid].fanout++;
    }
  }

  for (uint64_t oid = 0; oid < garbled_circuit->output_size; oid++) {
    int64_t gid = garbled_circuit->outputs[oid] - gate_bias;
    garbled_circuit->garbledGates[gid].fanout++;
  }

  return SUCCESS;
}

void ReduceFanout(const GarbledCircuit& garbled_circuit, int *fanout,
                  int64_t wid, int64_t gate_bias) {
  int64_t gid = wid - gate_bias;
  if (gid >= 0 && gid < (int64_t) garbled_circuit.gate_size) {
    fanout[gid]--;
    if (fanout[gid] == 0) {
      ReduceFanout(garbled_circuit, fanout,
                   garbled_circuit.garbledGates[gid].input0, gate_bias);
      ReduceFanout(garbled_circuit, fanout,
                   garbled_circuit.garbledGates[gid].input1, gate_bias);
    }
  }
}

