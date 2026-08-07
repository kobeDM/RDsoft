# 2026-08-07 livetime計算の修正 (develop/livetime)

## 対象ファイル
- `source/CalcRnRate.cc`

## 背景・問題点
- `last_time_in_days` は、エネルギー選択カット（area/pulse height/negative veto）と
  veto区間除外を通過したイベントでのみ更新されていた。
- そのため、有効なイベント（選択条件を通過したイベント）が1つも無いラン・時間帯では
  `last_time_in_days` が0のままとなり、`total_live_time_days`（= livetime）が0日として
  計算・表示される問題があった。
  - `Livetime: 0.00 days` としてプロットに現れるため、実質livetimeが表示されないのと同義。
  - `h_spectrum`等の`Scale(1.0 / binwidth / total_live_time_days)`がゼロ除算になる副作用もあった。

## 変更内容
- イベントループの先頭（`tree->GetEntry(i)` 直後、選択カットより前）で
  `event_time_in_days` を計算し、無条件に `last_time_in_days` を更新するように変更。
  - これにより、選択カットを通過しない・veto区間内のイベントも「観測された最新イベント」
    として livetime の終端計算に反映されるようになった。
- 既存の `time_in_days`（カット通過後にのみ計算されていた変数）を `event_time_in_days` に統一し、
  ループ末尾の冗長な `last_time_in_days = time_in_days;` を削除。
- veto区間の livetime 控除ロジック（`GetEffectiveExposureDays`）自体は変更なし。
  引き続き `total_live_time_days = GetEffectiveExposureDays(0.0, last_time_in_days, runstarttime, exclude_ranges)`
  として、veto区間を差し引いた実効livetimeを計算する。

## 対応した要求
1. livetime = 測定開始から最新イベント観測までの経過時間 → 修正済み
2. veto区間はそこから控除 → 既存の`GetEffectiveExposureDays`で対応済み（変更なし）
3. 有効なイベントが無い場合にlivetimeがプロットに表示されない問題 → 修正済み
   （全イベントの最新タイムスタンプを使うようになったため、有効イベント0でも
   正しいlivetimeが計算・表示される）

## 動作確認
- `build/` にて `make CalcRnRate` でビルド成功を確認。
