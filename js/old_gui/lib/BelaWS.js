class BelaWS {

  constructor ( {ip='192.168.7.2', port=8888, address='address'} = {} ) {

    this.ws = undefined;
    this.reconnectIntervalId = undefined;

    // TODO: Rate limiting
    this.sendInterval = 100; // ms
    this.sendCount = 0;

    this.handshakeReceived = false;

    this.connect (ip, port, address, this.reconnectIntervalId);

  }

  connect (ip, port, address, reconnectIntervalId) {

    this.ws = new WebSocket (`ws://${ip}:${port}/${address}`);
    this.ws.binaryType = 'arraybuffer';

    var that = this;

    this.ws.onopen = function() {

      console.log ('[WS] Connected:\n', that.ws);

      if (reconnectIntervalId) {

        console.log ('[WS] Clearing reconnect interval ID:', reconnectIntervalId);
        clearInterval (reconnectIntervalId);

      }

    };

    this.ws.onerror = event => {
      console.log ('[WS] Error:\n', event);
    };

    this.ws.onmessage = event => {

      if ('data' in event) {

        if (event.data == 'handshake') {

          this.handshakeReceived = true;
          this.send ({ command: 'handshakeReply' });
          console.log ('[WS] Handshake received:', this.handshakeReceived + '. Sending reply.');

          that.onConnect();

        } else {

          this.onMessage (JSON.parse (event.data));

        }

      }

    };

    this.ws.onclose = event => {

      console.log ('[WS] Closed:\n', event);

      this.handshakeReceived = false;

      if (reconnectIntervalId == undefined) {

        reconnectIntervalId = setInterval(() => {

          console.log ('[WS] Reconnecting:', reconnectIntervalId);
          this.connect (ip, port, address, reconnectIntervalId);

        }, 1000);

      }

    };

  }

  reconnect (ip, port, address) {

    this.reconnectIntervalId = setInterval(() => {

      this.ws = new WebSocket (`ws://${ip}:${port}/${address}`);
      console.log ('[WS] Reconnecting to ', this);

    }, 1000);

  }

  onConnect() {
    console.log('[WS] Called onConnect (override this function)');
  }
  
  onMessage (data) {
    console.log('[WS] Called onMessage (override this function)');
  }

  send (msg) {
    // TODO: rate limiting?
    if (this.ws.readyState === 1) this.ws.send (JSON.stringify (msg));
  }

  connected() {
    return this.ws.readyState;
  }

  close() {

  }

}
