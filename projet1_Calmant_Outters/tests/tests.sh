#!/bin/bash
echo "Tous les tests sont executés sur des fichier aléatoires de 5120 octets"
./tests/simple_test.sh &
wait
./tests/simlink_L10D50_test.sh &
wait
./tests/simlink_L50D50_test.sh &
wait
./tests/simlink_L70D100_test.sh &
wait
echo "La batterie de test est terminée!"
