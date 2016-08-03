'use strict';

const ERRORS = {
  ERROR_ALREADY_EXISTS: 183
};

const LOG_LEVELS = {
  CRITICAL: 1,
  ERROR: 2,
  WARNING: 3,
  INFORMATION: 4,
  VERBOSE: 5
};

module.exports = {
  ETW: require('./build/Debug/etwtrace.node').ETW,
  LOG_LEVELS,
  ERRORS,
  run
};

function run() {
  const logger = new module.exports.ETW('mysession');
  if (!logger.start()) {
    if (logger.status() === ERRORS.ERROR_ALREADY_EXISTS) {
      if (!logger.stop() || !logger.start()) {
        console.log(`Error in trace session ${logger.status}`);
        process.exit(1);
      }
    }
  }

  if (!logger.enableProvider('{77754E9B-264B-4D8D-B981-E4135C1ECB0C}', LOG_LEVELS.VERBOSE)) {
    console.log(`Error in trace session ${logger.status}`);
    process.exit(1);
  }

  function logMessage(msg) {
    console.log(msg);
  }

  if (!logger.openTrace(logMessage)) {
    console.log(`Error in trace session ${logger.status}`);
    process.exit(1);
  }

  if (!logger.process()) {
    console.log(`Error in trace session ${logger.status}`);
    process.exit(1);
  }

  function closeTrace(e) {
    console.log('closing');
    logger.closeTrace();
    logger.disableProvider('{77754E9B-264B-4D8D-B981-E4135C1ECB0C}');
    logger.stop();
  }

  process.on('SIGINT', closeTrace);
  process.on('SIGBREAK', closeTrace);
  process.on('uncaughtException', closeTrace);  
}
