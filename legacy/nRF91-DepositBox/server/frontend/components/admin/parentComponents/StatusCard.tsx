import { Card } from "@mui/material"

type StatusCardProps = {
    children: JSX.Element
}

const StatusCard = (props:StatusCardProps) => {
    return(
        <Card sx={{width: "300px", backgroundColor: "#d9e8ee", mb: "5px", ml:"5px"}}>
            {props.children}
        </Card>
    )
}

export default StatusCard;