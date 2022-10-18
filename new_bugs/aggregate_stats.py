import json, sys, logging, os

def help_msg():
        return """Usage: %s PATH
Generate the statistics of KIT test reports aggregation.""" % (sys.argv[0])

def list_to_str(l, wid):
        return ' '.join([str(x).ljust(wid) for x in l])

def remove_non_ascii(string):
    return ''.join(char for char in string if ord(char) < 128)

if len(sys.argv) != 2:
        logging.critical("Invalid argument\n%s" % help_msg())
        exit(1)


p = sys.argv[1]
f = open(p, "r")
m = json.load(f)
keywords = [
        [
                # 1: ptype
                [
                        "/proc/net/ptype",
                ],
                [
                        "bind",
                        "socket",
                        "setsockopt$packet_fanout",
                ],
        ],
        [
                # 2: flowlabel/xmit
                [
                        "sendmsg$inet6",
                        "sendmmsg$inet6",
                        "sendto$l2tp6",
                ],
                [
                        "setsockopt$inet6_IPV6_FLOWLABEL_MGR",
                ],
        ],
        [
                # 3: rds
                [
                        "bind$inet:sock_sctp",
                        "bind$rds",
                ],
                [
                        "bind$inet:sock_sctp",
                        "bind$rds",
                ],
        ],
        [
                # 4: flowlabel/connect
                [
                        "connect$inet6",
                        "connect:sock_icmp6",
                ],
                [
                        "setsockopt$inet6_IPV6_FLOWLABEL_MGR",
                ],
        ],
        [
                # 5: /proc/net/sockstat, sock alloc
                [
                        "/proc/net/sockstat",
                ],
                [
                        "socket",
                ],
        ],
        [
                # 6: socket cookie
                [
                        "getsockopt$SO_COOKIE",
                ],
                [
                        "getsockopt$SO_COOKIE",
                ],
        ],
        [
                # 7: assoc id
                [
                        "SCTP_SOCKOPT_CONNECTX3",
                ],
                [
                        "sendto",
                        "SCTP_SOCKOPT_CONNECTX",
                        "SCTP_SOCKOPT_CONNECTX3",
                        "connect$inet6:sock_sctp6",
                        "SCTP_SOCKOPT_CONNECTX_OLD",
                        "connect$inet:sock_sctp",
                        "sendmmsg$inet_sctp:sock_sctp",
                        "sendto$inet:sock_sctp",
                        "sendmsg$inet6:sock_sctp6",
                ],
        ],
        [
                # 8: /proc/net/sockstat, proto mem alloc
                [
                        "/proc/net/sockstat",
                ],
                [
                        "syz_emit_ethernet",
                        "recvfrom",
                        "sendto",
                        "sendmsg",
                        "sendmmsg",
                ],
        ],
        [
                # 9: /proc/net/protocols, proto mem alloc
                [
                        "/proc/net/protocols",
                ],
                [
                        "sendto",
                        "sendmmsg",
                ],
        ],
]

total_r = 0
total_rs = 0
total_report = 0

agg_r_cnt = [0] * len(keywords)
agg_rs_cnt = [0] * len(keywords)
report_cnt = [0] * len(keywords)

selected_reports = []
for _ in range(9):
        selected_reports.append([])

for r_call_cls in m['r_call_clusters']:
        total_report += int(r_call_cls['size'])
        total_r += 1
        for s_call_cls in r_call_cls['s_call_clusters']:
                total_rs += 1

for r_call_cls in m['r_call_clusters']:
        r_call_name = r_call_cls['r_call_name']
        for i, bug_kw in enumerate(keywords):
                r_call_match = False
                for bug_r_call_kw in bug_kw[0]:
                        if bug_r_call_kw in r_call_name:
                                r_call_match = True
                                break
                if not r_call_match:
                        continue
                agg_r_cnt[i] += 1
                for s_call_cls in r_call_cls['s_call_clusters']:
                        s_call_name = s_call_cls['s_call_name']
                        for bug_s_call_kw in bug_kw[1]:
                                if bug_s_call_kw in s_call_name:
                                        agg_rs_cnt[i] += 1
                                        report_cnt[i] += int(s_call_cls['size'])
                                        if len(selected_reports[i]) < 3:
                                                selected_reports[i].append([
                                                s_call_name.replace('\u0000', ''), 
                                                r_call_name.replace('\u0000', ''), 
                                                s_call_cls['node_path_clusters'][0]['diff_val_clusters'][0]['names'][0]["name"].strip()
                                        ])

print("")
print("%-10s%-70s%-70s%-10s" % ("Bug ID", "Sender call", "Receiver call", "Test report"))
for i, reports in enumerate(selected_reports):
        for r in reports:
                print(f"{str(i+1) : <10}{r[0] : <70}{r[1] : <70}{r[2] : <20}")

print("")
print("                                 Bug ID")
print("                 ", list_to_str(range(1, 10), 3))
print("Filtered reports ", list_to_str(report_cnt, 3))
print("AGG-RS groups    ", list_to_str(agg_rs_cnt, 3))
print("AGG-R groups     ", list_to_str(agg_r_cnt, 3))

print("")
print("                  Total")
print("Filtered reports ", str(total_report).ljust(3))
print("AGG-RS groups    ", str(total_rs).ljust(3))
print("AGG-R groups     ", str(total_r).ljust(3))