SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"

export RDSW="${SCRIPT_DIR}"
export PATH="${RDSW}/scripts:${RDSW}/bin:${PATH}"