let linq = require("../../../../build/install/lib/").network;
const { from } = require("rxjs");
const { takeWhile, take ,switchMap} = require("rxjs/operators");
const fs = require("fs");

linq.events("heartbeat").pipe(
	takeWhile(linq.run(10)), 
	take(1),
	switchMap(e => {
	})
);
