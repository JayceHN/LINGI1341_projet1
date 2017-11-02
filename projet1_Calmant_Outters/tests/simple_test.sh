#!/bin/bash

# cleanup d'un test précédent
echo "(simple_test) cleanup"
rm -f tests/received_file tests/input_file tests/*.log

# Fichier au contenu aléatoire de 512 octets
echo "(simple_test) input_file aléatoire"
dd if=/dev/urandom of=tests/input_file bs=1 count=5120 &> /dev/null

# On lance le receiver et capture sa sortie standard
echo "(simple_test) lance receiver (5120 octets)"
./receiver -f tests/received_file :: 7777  2> tests/receiver.log &
receiver_pid=$!

cleanup()
{
    kill -9 $receiver_pid
    exit 0
}
trap cleanup SIGINT  # Kill les process en arrière plan en cas de ^-C

# On démarre le transfert
echo "(simple_test) lance le sender"
if ! ./sender localhost 7777 < tests/input_file 2> tests/sender.log ; then
  echo "(simple_test) Crash du sender!"
  cat tests/sender.log
  err=1  # On enregistre l'erreur
fi
echo "(simple_test) On attend que le receiver finisse!"
sleep 6 # On attend 6 seconde que le receiver finisse

if kill -0 $receiver_pid &> /dev/null ; then
  echo "(simple_test) Le receiver ne s'est pas arreté à la fin du transfert!"
  kill -9 $receiver_pid
  err=1
else  # On teste la valeur de retour du receiver
  if ! wait $receiver_pid ; then
    echo "(simple_test) Crash du receiver!"
    cat tests/receiver.log
    err=1
  fi
fi

# On vérifie que le transfert s'est bien déroulé
if [[ "$(md5sum tests/input_file | awk '{print $1}')" != "$(md5sum tests/received_file | awk '{print $1}')" ]]; then
  echo "(simple_test) Le transfert simple_test a corrompu le fichier!"
  echo "(simple_test) Diff binaire des deux fichiers: (attendu vs produit)"
  diff -C 9 <(od -Ax -t x1z tests/input_file) <(od -Ax -t x1z tests/received_file)
  exit 1
else
  echo "simple_test est réussi!"
  exit ${err:-0}  # En cas d'erreurs avant, on renvoie le code d'erreur
fi
