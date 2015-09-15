
# flon-{listener|dispatcher|executor|invoker}

[![Join the chat at https://gitter.im/flon-io/flon](https://badges.gitter.im/Chat.svg)](https://gitter.im/flon-io/flon?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

The processes that make up a flon execution point.


## Components

* flon-executor
* flon-tasker
* flon-dispatcher
* flon-listener
* flon

## Testing

* `make clean sc` to test the code common to all Flon components
* `make clean sd` to test the dispatcher
* `make clean sx` to test the executor
* `make clean sl` to test the listener
* `make clean st` to test the tasker
* `make clean sn` to test individual radial instructions
* `make clean sz` to run a-to-z tests

* `make spec` to run all the spec in one go


## License

MIT (see [LICENSE.txt](LICENSE.txt))

