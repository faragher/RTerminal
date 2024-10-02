// Copyright (C) 2024, Michael Faragher
// Based on the RNode Firmware / Reticulum Network Stack by Mark Qvist

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef RNSCRYPTO
#define RNSCRYPTO

#include <SHA256.h>

void GetNameHash(byte Buffer[10], void* FullName, int Len) {
  SHA256 S;
  //S.reset();
  S.update(FullName, Len);
  S.finalize(Buffer, 10);
}

void GetIDfromPubKeys(byte Buffer[16], byte PubKey[32], byte PubSignKey[32]) {
  SHA256 S;
  S.update(PubKey, 32);
  S.update(PubSignKey, 32);
  S.finalize(Buffer, 16);
}

void GetDestinationFromIDandNameHash(byte Buffer[16], byte Identity[16], byte NameHash[10]) {
  SHA256 S;
  S.update(NameHash, 10);
  S.update(Identity, 16);
  S.finalize(Buffer, 16);
}

#endif
