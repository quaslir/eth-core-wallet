const script = document.createElement("script");
script.src = chrome.runtime.getURL("inject.js");
script.onload = () => script.remove();
(document.head || document.documentElement).appendChild(script);

window.addEventListener("message", (event) => {
  if (event.source !== window) return;
  const { type, id, method, params } = event.data || {};
  if (type !== "ETH_CORE_WALLET_REQUEST") return;

  chrome.runtime.sendMessage(
    {
      type: "FETCH_RPC",
      payload: { jsonrpc: "2.0", id, method, params },
    },
    (response) => {
      if (chrome.runtime.lastError || !response.success) {
        window.postMessage(
          {
            type: "ETH_CORE_WALLET_RESPONSE",
            id,
            error:
              "Wallet app not running: " +
              (chrome.runtime.lastError?.message || response?.error),
          },
          "*",
        );
        return;
      }

      window.postMessage(
        {
          type: "ETH_CORE_WALLET_RESPONSE",
          id,
          result: response.data.result,
          error: response.data.error?.message,
        },
        "*",
      );
    },
  );
});
