messfiles=`find QMAIL/queue/mess/* -type f | wc -l`
todofiles=`find QMAIL/queue/todo -type f | wc -l`
echo "messages in queue:" $messfiles
echo "messages in queue but not yet preprocessed:" $todofiles
