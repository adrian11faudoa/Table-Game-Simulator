#!/usr/bin/env bash
# =====================================================
# InfinityTable AWS Deployment Script
# Usage: ./deploy.sh [create|update|delete|status]
# =====================================================
set -euo pipefail

STACK_NAME="InfinityTable-GameServer"
REGION="${AWS_REGION:-us-east-1}"
TEMPLATE="$(dirname "$0")/../cloudformation/infinitytable-stack.yaml"

# ── Validate inputs ───────────────────────────────────
if [ -z "${KEY_NAME:-}" ]; then
    echo "ERROR: Set KEY_NAME environment variable."
    echo "  export KEY_NAME=my-keypair"
    exit 1
fi

MAX_PLAYERS="${MAX_PLAYERS:-6}"
INSTANCE_TYPE="${INSTANCE_TYPE:-c5.xlarge}"
SERVER_PASSWORD="${SERVER_PASSWORD:-}"

PARAMS="ParameterKey=KeyName,ParameterValue=${KEY_NAME} ParameterKey=MaxPlayers,ParameterValue=${MAX_PLAYERS} ParameterKey=InstanceType,ParameterValue=${INSTANCE_TYPE} ParameterKey=ServerPassword,ParameterValue=${SERVER_PASSWORD}"

ACTION="${1:-create}"

case "$ACTION" in

  create)
    echo ">>> Creating stack: $STACK_NAME in $REGION"
    aws cloudformation create-stack \
      --stack-name "$STACK_NAME" \
      --template-body "file://${TEMPLATE}" \
      --parameters $PARAMS \
      --capabilities CAPABILITY_IAM CAPABILITY_NAMED_IAM \
      --region "$REGION"

    echo "Waiting for stack creation..."
    aws cloudformation wait stack-create-complete \
      --stack-name "$STACK_NAME" --region "$REGION"

    echo ""
    echo "=== Stack created! ==="
    aws cloudformation describe-stacks \
      --stack-name "$STACK_NAME" --region "$REGION" \
      --query 'Stacks[0].Outputs' --output table
    ;;

  update)
    echo ">>> Updating stack: $STACK_NAME"
    aws cloudformation update-stack \
      --stack-name "$STACK_NAME" \
      --template-body "file://${TEMPLATE}" \
      --parameters $PARAMS \
      --capabilities CAPABILITY_IAM CAPABILITY_NAMED_IAM \
      --region "$REGION"

    echo "Waiting for update..."
    aws cloudformation wait stack-update-complete \
      --stack-name "$STACK_NAME" --region "$REGION"
    echo "Updated!"
    ;;

  delete)
    echo ">>> Deleting stack: $STACK_NAME"
    read -rp "Are you sure? (yes/no): " CONFIRM
    if [ "$CONFIRM" == "yes" ]; then
      aws cloudformation delete-stack \
        --stack-name "$STACK_NAME" --region "$REGION"
      echo "Waiting for deletion..."
      aws cloudformation wait stack-delete-complete \
        --stack-name "$STACK_NAME" --region "$REGION"
      echo "Stack deleted."
    else
      echo "Cancelled."
    fi
    ;;

  status)
    echo ">>> Stack status:"
    aws cloudformation describe-stacks \
      --stack-name "$STACK_NAME" --region "$REGION" \
      --query 'Stacks[0].{Status:StackStatus,Created:CreationTime}' \
      --output table
    echo ""
    echo ">>> Outputs:"
    aws cloudformation describe-stacks \
      --stack-name "$STACK_NAME" --region "$REGION" \
      --query 'Stacks[0].Outputs' --output table
    ;;

  logs)
    echo ">>> Fetching server logs from CloudWatch..."
    aws logs tail /infinitytable/server --follow --region "$REGION"
    ;;

  ssh)
    IP=$(aws cloudformation describe-stacks \
      --stack-name "$STACK_NAME" --region "$REGION" \
      --query 'Stacks[0].Outputs[?OutputKey==`ServerPublicIP`].OutputValue' \
      --output text)
    echo "Connecting to $IP..."
    ssh -i "${KEY_NAME}.pem" -o StrictHostKeyChecking=no "ec2-user@${IP}"
    ;;

  validate)
    echo ">>> Validating CloudFormation template..."
    aws cloudformation validate-template \
      --template-body "file://${TEMPLATE}" --region "$REGION"
    echo "Template is valid!"
    ;;

  *)
    echo "Usage: $0 [create|update|delete|status|logs|ssh|validate]"
    exit 1
    ;;
esac
