#!/bin/bash

# cleanup d'un test précédent
echo "(simlink_L70D150_test) cleanup"
rm -f tests/received_file tests/input_file

# Fichier au contenu aléatoire de 512 octets
echo "(simlink_L70D150_test) input file aléatoire"
dd if=/dev/urandom of=tests/input_file bs=1 count=5120 &> /dev/null

# On lance le simulateur de lien avec 10% de pertes et un délais de 50ms
echo "(simlink_L70D150_test) limsink 70% de pertes et un délais de 100ms (5120 octets)"
./tests/link_sim -p 26000 -P 7777 -l 70 -d 100 -R  &> tests/link.log &
link_pid=$!

# On lance le receiver et capture sa sortie standard
echo "(simlink_L70D150_test) lance le receiver"
./receiver -f tests/received_file :: 7777  2> tests/receiver.log &
receiver_pid=$!

cleanup()
{
    kill -9 $receiver_pid
    kill -9 $link_pid
    exit 0
}
trap cleanup SIGINT  # Kill les process en arrière plan en cas de ^-C

# On démarre le transfert
echo "(simlink_L70D150_test) lance le sender"
if ! ./sender ::1 26000 < tests/input_file 2> tests/sender.log ; then
  echo "(simlink_L70D150_test) Crash du sender!"
  cat tests/sender.log
  err=1  # On enregistre l'erreur
fi

sleep 5 # On attend 5 seconde que le receiver finisse

if kill -0 $receiver_pid &> /dev/null ; then
  echo "(simlink_L70D150_test) Le receiver ne s'est pas arreté à la fin du transfert!"
  kill -9 $receiver_pid
  err=1
else  # On teste la valeur de retour du receiver
  if ! wait $receiver_pid ; then
    echo "(simlink_L70D150_test) Crash du receiver!"
    cat tests/receiver.log
    err=1
  fi
fi

# On arrête le simulateur de lien
kill -9 $link_pid &> /dev/null

# On vérifie que le transfert s'est bien déroulé
if [[ "$(md5sum tests/input_file | awk '{print $1}')" != "$(md5sum tests/received_file | awk '{print $1}')" ]]; then
  echo "(simlink_L70D150_test) Le transfert a corrompu le fichier!"
  echo "(simlink_L70D150_test) Diff binaire des deux fichiers: (attendu vs produit)"
  diff -C 9 <(od -Ax -t x1z tests/input_file) <(od -Ax -t x1z tests/received_file)
  exit 1
else
  echo "simlink_L70D150_test est réussi!"
  exit ${err:-0}  # En cas d'erreurs avant, on renvoie le code d'erreur
fi
