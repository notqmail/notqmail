cd QMAILQUEUE
messdirs=`echo mess/* | wc -w`
messfiles=`find mess/* -print | wc -w`
tododirs=`echo todo | wc -w`
todofiles=`find todo -print | wc -w`
echo messages in queue: `expr $messfiles - $messdirs`
echo messages in queue but not yet preprocessed: `expr $todofiles - $tododirs`
