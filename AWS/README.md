# InfinityTable — AWS Deployment Guide

## Architecture

```
Internet
    |
[Elastic IP]
    |
[EC2: c5.xlarge]  ←── game clients connect here (UDP:7777)
    |
    ├── Docker: infinitytable/server  (port 7777 UDP)
    ├── Nginx:  health check          (port 80 TCP)
    └── S3:     save files + backups
```

## Quick Deploy (5 minutes)

### Prerequisites
- AWS CLI configured (`aws configure`)
- An EC2 Key Pair created in your target region
- Docker Hub account (to push your server image)

### Step 1 — Set environment variables
```bash
export AWS_REGION=us-east-1
export KEY_NAME=my-keypair
export MAX_PLAYERS=6
export INSTANCE_TYPE=c5.xlarge   # c5.xlarge = ~$0.17/hr
export SERVER_PASSWORD=""         # blank = public server
```

### Step 2 — Deploy
```bash
chmod +x AWS/scripts/deploy.sh
./AWS/scripts/deploy.sh create
```

### Step 3 — Connect
After ~5 minutes, the script prints:
```
ConnectionString  |  54.123.45.67:7777
SSHCommand        |  ssh -i my-keypair.pem ec2-user@54.123.45.67
HealthCheckURL    |  http://54.123.45.67/health
```

Open InfinityTable → Browse Sessions → Direct Connect → paste IP:7777

---

## Instance Type Guide

| Type       | vCPU | RAM  | Cost/hr | Players | Notes            |
|------------|------|------|---------|---------|------------------|
| t3.medium  | 2    | 4GB  | $0.042  | 2-3     | Dev/testing only |
| c5.large   | 2    | 4GB  | $0.085  | 4-6     | Budget production |
| c5.xlarge  | 4    | 8GB  | $0.170  | 6-10    | **Recommended**  |
| c5.2xlarge | 8    | 16GB | $0.340  | 10-20   | Heavy mod loads  |

---

## Management Commands

```bash
# Check stack status and outputs
./AWS/scripts/deploy.sh status

# SSH into server
./AWS/scripts/deploy.sh ssh

# Tail server logs in real time
./AWS/scripts/deploy.sh logs

# Update stack (e.g. change instance type)
INSTANCE_TYPE=c5.2xlarge ./AWS/scripts/deploy.sh update

# Validate template before deploying
./AWS/scripts/deploy.sh validate

# Tear down everything
./AWS/scripts/deploy.sh delete
```

## Manual Server Management (via SSH)

```bash
# View server logs
sudo journalctl -u infinitytable -f

# Restart server
sudo systemctl restart infinitytable

# Check Docker containers
sudo docker ps
sudo docker logs it_server --tail 50 -f

# Force save sync to S3
sudo docker exec it_save_sync aws s3 sync /saves s3://$S3_BUCKET/saves/

# Update server image
sudo docker pull infinitytable/server:latest
sudo systemctl restart infinitytable
```

## Auto-Scaling (Advanced)

For high-traffic deployments, replace the single EC2 with an Auto Scaling Group:

```yaml
# Add to CloudFormation:
GameServerASG:
  Type: AWS::AutoScaling::AutoScalingGroup
  Properties:
    MinSize: 1
    MaxSize: 5
    DesiredCapacity: 1
    TargetGroupARNs:
      - !Ref GameServerTargetGroup
    # Each instance = one game server room
    # Use a Network Load Balancer (TCP/UDP) in front
```

## Cost Estimate (c5.xlarge, us-east-1, 24/7)

| Resource       | Monthly Cost |
|----------------|-------------|
| EC2 c5.xlarge  | ~$123       |
| EIP            | ~$3.60      |
| S3 (10GB)      | ~$0.23      |
| CloudWatch     | ~$3.00      |
| Data transfer  | ~$5.00      |
| **Total**      | **~$135/mo**|

Use a Spot Instance to save 60-70%:
```bash
export INSTANCE_TYPE=c5.xlarge
# Add to CloudFormation InstanceMarketOptions:
#   MarketType: spot
#   SpotOptions: { MaxPrice: "0.07", SpotInstanceType: persistent }
```
