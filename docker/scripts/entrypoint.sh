#!/bin/sh

LOGDIR=/opt/spt/logs

Defaults()
{
  if [ -z "$PORT" ]
  then
    PORT=2030
    echo "PORT not set.  Will default to $PORT"
  fi

  if [ -z "$THREADS" ]
  then
    THREADS=4
    echo "THREADS not set.  Will default to $THREADS"
  fi

  if [ -z "$LOG_LEVEL" ]
  then
    LOG_LEVEL="info"
    echo "LOG_LEVEL not set.  Will default to $LOG_LEVEL"
  fi
}

Service()
{
  echo "Starting encrypter service"
  /opt/spt/bin/encrypter-service --console \
    --log-dir ${LOGDIR}/ --log-level $LOG_LEVEL \
    --port $PORT --threads $THREADS
}

if [ -z "$RUN_SERVER" ]
then
  /opt/spt/bin/encrypter $*
  exit 0
fi

Defaults && Service