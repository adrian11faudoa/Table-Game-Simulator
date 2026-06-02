#!/usr/bin/env bash
# =====================================================
# InfinityTable Save Backup to S3
# Add to crontab: */5 * * * * /opt/infinitytable/backup_saves.sh
# =====================================================
set -euo pipefail

SAVES_DIR="/opt/infinitytable/Saved"
S3_BUCKET="${S3_BUCKET:-infinitytable-saves}"
REGION="${AWS_DEFAULT_REGION:-us-east-1}"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)

# Sync saves to S3
aws s3 sync "$SAVES_DIR" "s3://${S3_BUCKET}/saves/" \
    --region "$REGION" \
    --quiet \
    --exclude "*.tmp"

# Daily full backup with timestamp
if [ "$(date +%H%M)" = "0300" ]; then
    aws s3 cp "$SAVES_DIR/InfinityTable.db" \
        "s3://${S3_BUCKET}/backups/${TIMESTAMP}_InfinityTable.db" \
        --region "$REGION"
    echo "[$(date)] Daily backup uploaded: ${TIMESTAMP}_InfinityTable.db"
fi
