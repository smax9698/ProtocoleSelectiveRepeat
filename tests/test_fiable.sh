#!/bin/bash

# cleanup d'un test précédent
#rm -f received_file input_file

# On lance le simulateur de lien. Lien fiable
./link_sim -p 1341 -P 2456  &> link.log &
link_pid=$!

# On lance le receiver et capture sa sortie standard
./../src/receiver :: 2456  > test_out_fiable.txt 2> receiver.log &
receiver_pid=$!

cleanup()
{
    kill -9 $receiver_pid
    kill -9 $link_pid
    exit 0
}
trap cleanup SIGINT  # Kill les process en arrière plan en cas de ^-C

# On démarre le transfert
if ! ./../src/sender localhost 1341 < test_in.txt 2> sender.log ; then
  echo "Crash du sender!"
  cat sender.log
  err=1  # On enregistre l'erreur
fi

sleep 3 # On attend 8 seconde que le receiver finisse

if kill -0 $receiver_pid &> /dev/null ; then
  echo "Le receiver ne s'est pas arreté à la fin du transfert!"
  kill -9 $receiver_pid
  err=1
else  # On teste la valeur de retour du receiver
  if ! wait $receiver_pid ; then
    echo "Crash du receiver!"
    cat receiver.log
    err=1
  fi
fi

# On arrête le simulateur de lien
kill -9 $link_pid &> /dev/null
