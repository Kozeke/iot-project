var ubidots = require('ubidots');
var client = ubidots.createClient('BBFF-30f2df5cf4242abf0085394fb2ae7221720');

client.auth(function () {
    this.getDatasources(function (err, data) {
        console.log('datasources', data.results);
    });

    var ds = this.getDatasource('62caff6f1f7e0b2fea8c1d5e');

    ds.getVariables(function (err, data) {
        console.log('variables', data.results);
    });

    ds.getDetails(function (err, details) {
        console.log('details ds', details);
    });
    var v = this.getVariable('temperature');
    async function getVars() {
        return this.getVariable('temperature');

    };

    v.getDetails(function (err, details) {
        console.log('details v', details);
    })

    v.saveValue(22)

    v.getValues(function (err, data) {
        console.log('vals', data.results);
    });


});