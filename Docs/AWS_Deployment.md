# AWS Deployment Guide

## Quick Deploy

```bash
export KEY_NAME=my-keypair
export AWS_REGION=us-east-1
export MAX_PLAYERS=6
export INSTANCE_TYPE=c5.xlarge  # ~$0.17/hr

chmod +x AWS/scripts/deploy.sh
./AWS/scripts/deploy.sh create
```

After ~5 minutes:
```
ConnectionString  54.123.45.67:7777
HealthCheckURL    http://54.123.45.67/health
SSHCommand        ssh -i my-keypair.pem ec2-user@54.123.45.67
```

## Instance Guide

| Type       | Cost/hr | Players |
|------------|---------|---------|
| t3.medium  | $0.042  | 2-3 dev |
| c5.large   | $0.085  | 4-6     |
| c5.xlarge  | $0.170  | 6-10 ** |
| c5.2xlarge | $0.340  | 10-20   |

## Management

```bash
./AWS/scripts/deploy.sh status   # stack outputs
./AWS/scripts/deploy.sh logs     # CloudWatch log tail
./AWS/scripts/deploy.sh ssh      # SSH into server
./AWS/scripts/deploy.sh update   # update stack
./AWS/scripts/deploy.sh delete   # tear down
```
