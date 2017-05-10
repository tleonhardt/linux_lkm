# Character Device Driver with Mutex
The original character driver in ../char_dev wasn't thread or process safe.
It had a concurrency bug.

This minor variant uses a mutex to fix the issue.
