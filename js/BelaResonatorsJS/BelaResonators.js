import Bela from './Bela.js'

export default class BelaResonators extends Bela {
	constructor(ip='192.168.7.2') { super(ip) }

	setModel(index, model) {
		if (this.isConnected()) {
			this.control.send({ 
				command: 'updateModel',
				args: {
					index: index,
					model: model
				}
			})
		}
	}

	setPitch(index, pitch) {
		if (this.isConnected()) {
			this.control.send({ 
				command: 'updatePitch',
				args: {
					index: index,
					pitch: pitch
				}
			})
		}
	}

}
