#!/bin/sh

cd ../GameBoy/Debug

./gameboy BROKER CAUGHT_POKEMON 1 1
./gameboy BROKER CAUGHT_POKEMON 2 0

./gameboy BROKER NEW_POKEMON Pikachu 2 3 1

./gameboy BROKER CATCH_POKEMON Onyx 4 5

./gameboy SUSCRIPTOR NEW_POKEMON 10

./gameboy BROKER CATCH_POKEMON Charmander 4 5
