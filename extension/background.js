chrome.runtime.onMessage.addListener((message, sender, sendResponse) => {
  if (message.type !== "FETCH_RPC") return;

  fetch("http://127.0.0.1:8989/", {
    method: "POST",
    headers: {
      Origin: sender.origin || "",
      "Content-Type": "application/json",
    },
    body: JSON.stringify(message.payload),
  })
    .then((res) => res.json())
    .then((data) => sendResponse({ success: true, data }))
    .catch((err) => sendResponse({ success: false, error: err.message }));

  return true;
});
