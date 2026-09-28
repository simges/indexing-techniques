🏛️ System Architecture & Data Structures

                 LinearHash
         ┌────────────────────────┐
         │ _table_size: 2         │
         │ _bucket_size: 2        │
         │ _round: 0              │
         │ _split_ptr ─────────┐  │
         └─────────────────────┼──┘
                               │
            Hash Table Index   │   Buckets (std::list)
           ┌────────────────┐  │  ┌─────────────────┐
  idx 0    │   Bucket 0     │◄─┴──┤ [ Key 4, Key 8 ]│
           ├────────────────┤     ├─────────────────┤
  idx 1    │   Bucket 1     │     │ [ Key 1, Key 9 ]│
           ├────────────────┤     ├─────────────────┤
  idx 2    │   Bucket 2     │     │ [ Key 2, Key 10]│
           └────────────────┘     └─────────────────┘
                   │                       │
                   ▼                       ▼
            Overflow Bitmap      Tracks overflow state
          ┌────────────────┐     via 64-bit block masks
          │ [ 0 | 0 | 1 ]  │
