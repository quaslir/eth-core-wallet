(function () {
  let request_id = 0;
  const pending = new Map();

  window.addEventListener("message", (event) => {
    if (event.source !== window) return;
    const { type, id, result, error } = event.data || {};
    if (type !== "ETH_CORE_WALLET_RESPONSE") return;
    const resolver = pending.get(id);
    if (!resolver) return;
    pending.delete(id);
    error ? resolver.reject(new Error(error)) : resolver.resolve(result);
  });

  function request({ method, params }) {
    return new Promise((resolve, reject) => {
      const id = ++request_id;
      pending.set(id, { resolve, reject });
      window.postMessage(
        {
          type: "ETH_CORE_WALLET_REQUEST",
          id,
          method,
          params,
        },
        "*",
      );
    });
  }

  const provider = {
    isEthCoreWallet: true,
    isMetaMask: true,
    request,

    on(event, handler) {
      console.log("[ETH Core] dApp subscribed to: " + event);
    },
    removeListener(event, handler) {},
    removeEventListener(event, handler) {},
    removeAllListeners(event) {},
  };

  if (!window.ethereum) window.ethereum = provider;

  const info = {
    uuid: "eth-core-wallet-" + crypto.randomUUID(),
    name: "ETH Core Wallet",
    icon: "data:image/svg+xml;base64,PHN2Zy8+",
    rdns: "com.ethcorewallet",
  };

  function announce() {
    window.dispatchEvent(
      new CustomEvent("eip6963:announceProvider", {
        detail: Object.freeze({ info, provider }),
      }),
    );
  }

  window.addEventListener("eip6963:requestProvider", announce);
  announce();
})();
