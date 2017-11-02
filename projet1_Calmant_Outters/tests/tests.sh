#!/bin/bash
./tests/simple_test.sh &
wait
./tests/simlink_L10D50_test.sh &
wait
echo "La batterie de test est terminée!"
