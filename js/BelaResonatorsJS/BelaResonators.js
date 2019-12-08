import Bela from './Bela.js'

export default class BelaResonators extends Bela {
	constructor(ip='192.168.7.2') { super(ip) }

	setModel(model) {
		if (this.isConnected()) {
			this.control.send({ 
				command: 'setModel', 
				args: model 
			})
		}
	}

	setModelAtIndex(index, model) {
		if (this.isConnected()) {
			this.control.send({ 
				command: 'setModel', 
				args: {
					index: index,
					model: model
				}
			})
		}
	}

	setResonatorParamAtBankIndex(bankIndex, resIndex, paramIndex, value) {
		if (this.isConnected()) {
			this.control.send({ 
				command: 'setResParamAtBankIndex', 
				args: {
					bankIndex:  bankIndex, 
					resIndex:   resIndex, 
					paramIndex: paramIndex, 
					value:      value
				}
			})
		}
	}

	setResonatorAtBankIndex(bankIndex, resIndex, params) {
		if (this.isConnected()) {
			this.control.send({ 
				command: 'setResAtBankIndex', 
				args: {
					bankIndex: bankIndex, 
					resIndex:  resIndex, 
					params:    params
				}
			})
		}
	}

  //   setResonatorsAtBankIndex(bankIndex, resIndices, paramVects) {
  //   	// paramVects = [{f, g, d}];
  //   	// paramVects = []
  //   	if (this.isConnected()) {
		// 	this.control.send({ 
		// 		command: 'setRessAtBankIndex', 
		// 		args: {
		// 			bankIndex:  bankIndex, 
		// 			resIndices: resIndices, 
		// 			paramVects: paramVects
		// 		}
		// 	})
		// }
  //   }

}
