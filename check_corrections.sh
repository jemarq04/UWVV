for pog in data/XPOG/*/; do
  for era in ${pog}*/; do
    for f in ${era}*; do
      [[ ! $f = *.json.gz ]] && continue
      [[ -L $f ]] && continue

      centralfile="/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/${f#data\/XPOG\/}"
      if ! diff $f $centralfile >& /dev/null; then
        echo $f is out-of-date.
        read -p "Update the file? (y/n): " choice
        [[ $choice == [yY] ]] && cp -v $centralfile $f
      fi
    done
  done
done
