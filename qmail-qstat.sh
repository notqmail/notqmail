cd QMAIL
echo messages in queue: `find queue/mess -type f -print | wc -l`
echo messages in queue but not yet preprocessed: `find queue/todo -type f -print | wc -l`
