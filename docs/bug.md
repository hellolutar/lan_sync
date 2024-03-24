1. 内存泄漏
   - ne没有delete
   - server端保存的connctx没有delete
2. 不清楚原因的bug
   - cli断开，有时会出现server崩溃
   - 不知道为什么reqRs时，server回复消息的频率==syncReIdx的频率