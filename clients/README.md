# TODO
Decide what to do about `kvclient.cpp`/`kv.cpp` (and required classes in `./utils/*.hpp`).

- One of them is unused and can be deleted

The original purpose was:
- Test the server at high query rate
- A client runs locally
- One or more remote clients ran on Azure VMs
- The local client sends a message to the remote clients, which would then begin firing queries at the server (also in Azure), reporting the timing


Options:
1. Replace with Python:
    - Unlikely able to compete with queries/s
    - Can run more remote client instances to negate peformance
    - Far easier to maintain
    - Avoids dragging in boost::beast/boost::asio and dependencies
    
2. Continue
    - Remote clients' servers can be replaced with uWebSockets
    - uWebSockets does not offer client, so decide to replace boost::beast


<br/>
<br/>

__Option 1__ is tempting.
