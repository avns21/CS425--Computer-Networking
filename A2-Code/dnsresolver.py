import dns.message
import dns.query
import dns.rdatatype
import dns.resolver
import time

# Root DNS servers used to start the iterative resolution process
ROOT_SERVERS = {
    "198.41.0.4": "Root (a.root-servers.net)",
    "199.9.14.201": "Root (b.root-servers.net)",
    "192.33.4.12": "Root (c.root-servers.net)",
    "199.7.91.13": "Root (d.root-servers.net)",
    "192.203.230.10": "Root (e.root-servers.net)"
}

TIMEOUT = 3  # Timeout in seconds for each DNS query attempt

def send_dns_query(server, domain):
    """
    Sends a DNS query to the given server for an A record of the specified domain.
    Returns the response if successful, otherwise returns None.
    """
    try:
        query = dns.message.make_query(domain, dns.rdatatype.A)  # Construct the DNS query
        # Modified Code 1 - Send the query using UDP 
        response = dns.query.udp(query, server, timeout=TIMEOUT)  # Sends the DNS query using UDP protocol to the given server
        return response  # Returns the response received from the server

    except Exception:
        return None  # If an error occurs (timeout, unreachable server, etc.), return None

def extract_next_nameservers(response):
    """
    Extracts nameserver (NS) records from the authority section of the response.
    Then, resolves those NS names to IP addresses.
    Returns a list of IPs of the next authoritative nameservers.
    """
    ns_ips = []  # List to store resolved nameserver IPs
    ns_names = []  # List to store nameserver domain names

    # Loop through the authority section to extract NS records
    for rrset in response.authority:
        if rrset.rdtype == dns.rdatatype.NS:
            for rr in rrset:
                ns_name = rr.to_text()
                ns_names.append(ns_name)  # Extract nameserver hostname
                print(f"Extracted NS hostname: {ns_name}")

    # Modified Code 2 - Resolve the extracted NS hostnames to IP addresses

    # Resolve NS names to IPs
    for ns_name in ns_names:  # Iterate through the extracted nameserver hostnames
        try:
            answer = dns.resolver.resolve(ns_name, "A")  # Resolve NS hostname to an IP address
            for rdata in answer:  # Loop through the resolved IP addresses
                ns_ips.append(rdata.to_text())  # Store the resolved IP in the list
        except Exception as e:  # Handle cases where resolution fails
            print(f"[ERROR] Could not resolve NS {ns_name}: {e}") 
            return None # Return None if any NS resolution fails
    
    return ns_ips  # Return list of resolved nameserver IPs 


def iterative_dns_lookup(domain):
    """
    Performs an iterative DNS resolution starting from root servers.
    """
    print(f"[Iterative DNS Lookup] Resolving {domain}")
    next_ns_list = list(ROOT_SERVERS.keys())  # Start with the root server IPs
    stage = "ROOT"  # Track resolution stage (ROOT, TLD, AUTH)

    while next_ns_list:
        ns_ip = next_ns_list.pop(0)  # Pick the first available nameserver to query
        response = send_dns_query(ns_ip, domain)

        if response:  # Checks if response is not NONE
            print(f"[DEBUG] Querying {stage} server ({ns_ip}) - SUCCESS")

            # If an answer is found, print and return
            if response.answer:
                print(f"[SUCCESS] {domain} -> {response.answer[0][0]}")
                return

            # If no answer, extract the next set of nameservers
            next_ns_list = extract_next_nameservers(response)

            # Modified Code 3 - Move to the next resolution stage, i.e., it is either TLD, ROOT, or AUTH
            
            # Determine next resolution stage
            if stage == "ROOT":
                stage = "TLD"   # Move to Top-Level Domain (TLD) servers
            elif stage == "TLD":
                stage = "AUTH"  # Move to authoritative name servers

        else:
            print(f"[ERROR] Query failed for {stage} {ns_ip}")
            # Don't return.  Try the next nameserver in the list
            # return  # Stop resolution if a query fails

    print("[ERROR] Resolution failed.")  # Final failure message if no nameservers respond

def recursive_dns_lookup(domain):
    """
    Performs recursive DNS resolution using the system's default resolver.
    """
    print(f"[Recursive DNS Lookup] Resolving {domain}")
    try:
        # Modified Code 4 - Perform recursive resolution using the system's DNS resolver
        # Notice that the next line is looping through, therefore you should have something like answer = ??

        # First, resolve the domain's nameservers (NS records)
        answer = dns.resolver.resolve(domain, "NS")   # Resolves NS records to get authoritative nameservers
        for rdata in answer:
            print(f"[SUCCESS] {domain} -> {rdata}")   # Prints the authoritative nameservers

        # Then, resolve the domain's A records (IPv4 addresses)
        answer = dns.resolver.resolve(domain, "A")    # Resolves A records to get the IP addresses
        for rdata in answer:
            print(f"[SUCCESS] {domain} -> {rdata}")   # Prints the resolved IP addresses
    except Exception as e:
        print(f"[ERROR] Recursive lookup failed: {e}")  # Handle resolution failure

if __name__ == "__main__":
    import sys

    if len(sys.argv) != 3 or sys.argv[1] not in {"iterative", "recursive"}:
        print("Usage: python3 dns_resolver.py <iterative|recursive> <domain>")
        sys.exit(1)

    mode = sys.argv[1]  # Get mode (iterative or recursive)
    domain = sys.argv[2]  # Get domain to resolve

    start_time = time.time()  # Record start time

    # Execute the selected DNS resolution mode
    if mode == "iterative":
        iterative_dns_lookup(domain)
    else:
        recursive_dns_lookup(domain)

    print(f"Time taken: {time.time() - start_time:.3f} seconds")  # Print execution time
