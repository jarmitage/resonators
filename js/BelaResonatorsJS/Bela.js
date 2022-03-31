import BelaData    from './BelaData.js'
import BelaControl from './BelaControl.js'

export default class Bela {
	constructor(ip='192.168.7.2') {
		this.port = 5555
		this.addresses = {
			data:    'gui_data',
			control: 'gui_control'
		}
		this.ip = ip
		this.data    = new BelaData    (this.port, this.addresses.data,    this.ip)
		this.control = new BelaControl (this.port, this.addresses.control, this.ip)
	}

	isConnected() {
		let dataState    = this.data.isConnected()
		let controlState = this.control.isConnected()
		if (dataState && controlState) return true
		else return false
	}

	// onConnect() {
		
	// }

}
