export default class BelaWebSocket {
    constructor(port, address, ip = location.host) {
        this.port    = port;
        this.address = address;
        this.ip      = ip;
        this.ws;
        this.connectInterval = 1500;
        this.url = "ws://" + this.ip + ":"+this.port+"/"+this.address;
        this.connect(this.url);
        this.connectionState = false;
    }

    connect(url) {
        this.ws = new WebSocket(url);
        this.ws.onopen    = this.onOpen.bind(this);
        this.ws.onclose   = this.onClose.bind(this);
        this.ws.onerror   = this.onError.bind(this);
        this.ws.onmessage = this.onMessage.bind(this);
    };

    reconnect(connectInterval) {
        setTimeout(() => {
            console.log("[BelaWebSocket] reconnect(): Retrying connection in %d ms\n", connectInterval);
            try {
                this.ws = new WebSocket(this.ws.url);
            } catch (e) {
                console.log("[BelaWebSocket] reconnect(): Error", e)
            }
            this.ws.onopen    = this.onOpen.bind(this);
            this.ws.onclose   = this.onClose.bind(this);
            this.ws.onerror   = this.onError.bind(this);
            this.ws.onmessage = this.onMessage.bind(this);
        }, connectInterval)
    };

    onClose(event) {
        console.log("[BelaWebSocket] onClose(): Socket closed")
        this.connectionState = false
        if(event.code != 1000) this.reconnect(this.connectInterval);
    }

    onError(err){
        console.log("[BelaWebSocket] onError(): " + err);
        switch (err.code){
    		case 'ECONNREFUSED':
    			this.reconnect(this.connectInterval);
    			break;
    		default:
    			break;
        }
    }

    onOpen(){
        console.log("[BelaWebSocket] onOpen(): opened with URL", this.ws.url);
        this.connectionState = true
        this.ws.binaryType = 'arraybuffer';
        this.ws.onclose = this.onClose.bind(this);
        this.ws.onerror = this.onError.bind(this);
    }

    onMessage (msg) {
        let data = msg.data;
        let parsedData = isJson(data);

        if(parsedData) {
            if(event.data == 'connection') {
                let obj = { event: "connection-reply" }
                print("Connection reply: \n" + obj);
                obj = JSON.stringify(obj);
                if (this.ws.readyState === 1)
                    this.ws.send(obj);
            }
        }
        this.onData(data, parsedData);
    }

    onData (data, parsedData) {}

    isConnected() { return this.connectionState }

}

function isJson(str) {
	let data;
	try {
		data = JSON.parse(str);
		return data;
	} catch (e) {
		return false;
	}
}
