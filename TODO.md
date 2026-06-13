# TODO - DPI Engine Feature Enhancements

## Completed
- [x] Gather repo context (DPI pipeline, SNI extraction, app mapping, rules, config structure)

## Planned / In Progress
- [ ] Add `traffic_report.json` generation alongside output pcap (replace `.pcap` with `_report.json`).
- [ ] Track detected SNI/domain names; print Top 10 most frequent at end.
- [ ] Implement traffic_report.json generation and throughput stats.

- [ ] Add `log_level` field support (ERROR/INFO/DEBUG) parsing from config.json; wire into console logging.
- [ ] Extend application classification with: Reddit, LinkedIn, Twitch, ChatGPT, StackOverflow.
- [ ] Measure execution time; display packets/sec and MB/sec.

## Follow-ups
- [ ] Build + run on `test_dpi.pcap` to validate JSON/report/throughput/domain top10/app classification.

