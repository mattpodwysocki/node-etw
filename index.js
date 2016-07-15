'use strict';

const LOG_LEVELS = {
  CRITICAL: 1,
  ERROR: 2,
  WARNING: 3,
  INFORMATION: 4,
  VERBOSE: 5
};

class ETW {
  constructor(sessionName) {
    this.sessionName = sessionName;
  }

  start() {

  }

  stop() {

  }

  enableProvider(providerGuid, logLevel) {

  }

  disableProvider(providerGuid) {

  }

  openTrace(cb) {

  }

  closeTrace() {

  }

  process() {

  }
}

module.exports = {
  ETW,
  LOG_LEVELS
};
