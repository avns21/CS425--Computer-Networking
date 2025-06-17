# DNS Resolver

## Overview
This Python script implements both **iterative** and **recursive** DNS resolution. It queries DNS servers to resolve domain names to IP addresses using the **dnspython** library.

- **Iterative DNS Lookup**: Simulates how a real DNS resolver works by querying root servers, then TLD servers, and finally authoritative servers.
- **Recursive DNS Lookup**: Uses the system's default DNS resolver to fetch the IP address directly.

## Installation
Ensure you have **dnspython** installed before running the script:
```sh
pip install dnspython
```

## Usage
Run the script with the following command:
```sh
python3 dns_resolver.py <mode> <domain>
```
Where:
- `<mode>` is either `iterative` or `recursive`
- `<domain>` is the domain name to resolve (e.g., `google.com`)

Example:
```sh
python3 dns_resolver.py iterative google.com
```

---

## Code Flow
### Iterative DNS Lookup
**Function:** `iterative_dns_lookup(domain)`

1. **Initialize Root Servers**: The function starts with a predefined list of root DNS servers.
2. **Query the Root Server**: Sends a DNS request to a root server for an `A` record of the given domain.
3. **Process Response**:
   - If an `A` record is found, it prints the resolved IP and exits.
   - Otherwise, it extracts `NS` (Nameserver) records from the authority section.
4. **Query Next Nameservers**:
   - The extracted NS records are resolved to IPs.
   - The function repeats the process at the **TLD (Top-Level Domain)** servers, then the **authoritative servers**.
5. **Final Resolution**:
   - The process continues until it finds the IP or exhausts available nameservers.
   - If no response is obtained, an error message is displayed.

### Recursive DNS Lookup
**Function:** `recursive_dns_lookup(domain)`

1. **Query System's Resolver**: Uses the system's DNS resolver (e.g., Google DNS, Cloudflare, or local ISP resolver).
2. **Request NS Records**: The function first resolves the **NS records** (authoritative nameservers) for the domain.
3. **Request A Records**: Then, it queries for **A records** to fetch the IP address.
4. **Output Results**:
   - Prints both authoritative nameservers and resolved IP addresses.
   - If resolution fails, an error message is displayed.

---

## Comparison: Iterative vs. Recursive DNS Resolution
| Feature         | Iterative Resolution        | Recursive Resolution        |
|---------------|----------------------|----------------------|
| Query Type    | Client queries each server in sequence | Client relies on a recursive resolver |
| Control      | Client controls the full resolution process | Resolver performs the full lookup |
| Performance  | Slower (depends on multiple queries) | Faster (single request to system resolver) |
| Accuracy     | Mimics real-world DNS resolution | Uses cached or pre-resolved data |
| Use Case     | DNS server emulation, learning DNS flow | General internet browsing, fast lookups |

---

## Execution Time
At the end of execution, the script prints the total time taken:
```sh
Time taken: X.XXX seconds
```
This helps in comparing performance between iterative and recursive methods.

## Conclusion
- **Iterative mode** shows how DNS servers resolve domain names step by step.
- **Recursive mode** leverages existing resolvers for faster lookups.
- Understanding both methods helps in troubleshooting DNS issues and optimizing resolution strategies.

## Member Contributions
- **Mohammed Sibtain Ansari** (210616) (33.33%): Coding part of the assignment
- **Avinash Shukla** (210236) (33.33%): Debugging and adding comments to the changes made.
- **Mantapuram Shreeja** (210592) (33.33%): Created README file.

