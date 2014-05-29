#! /bin/sh

usage() {
  cat << EOF
This script will synchronise mailman subscibers with nominated Drupal groups.

OPTIONS:
 -h  Show this message
 -u  The mapping url (REQUIRED)
 -k  Secret key (REQUIRED
 -n  Dry run
 -c  Additional arguments to pass to curl
 -v  Verbose
EOF
}

while getopts "u:c:k:hvn" OPTION; do
  case $OPTION in
    h)
      usage
      exit 1
      ;;
    u)
      MAP=$OPTARG
      ;;
    c)
      CURL_ARGS=$OPTARG
      ;;
    k)
      KEY=$OPTARG
      ;;
    n)
      DRYRUN="-n"
      ;;
    v)
      VERBOSE=1
      ;;
    *)
      usage
      exit
      ;;
  esac
done

if [ -n "$MAP" ] && [ -n "$KEY" ]; then
  curl $CURL_ARGS --fail --silent --show-error "$MAP?mailman_key=$KEY" | while read mapping; do
    set $mapping

    # Check all arguments are present
    if [ -z "$1" ] || [ -x "$2" ]; then
      echo "Incorrect arguments supplied: $mapping"
      echo "Bailing all further synchronizations."
      exit 1
    fi

    if [ "$VERBOSE" = 1 ]; then
      echo "Synchronizing group $1"
    fi
    curl $CURL_ARGS --fail --silent --show-error "$2""?mailman_key=$KEY" | ifne /usr/lib/mailman/bin/sync_members $DRYRUN -f - $1
  done
else
  usage
fi
