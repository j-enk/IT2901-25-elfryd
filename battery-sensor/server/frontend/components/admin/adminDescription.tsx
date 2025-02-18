import { Typography } from "@mui/material"

type AdminDescriptionProps = {
    description:string
}

const AdminDescription = (props: AdminDescriptionProps) => {
    return(
        <Typography
        variant="subtitle1"
        textAlign="center"
        fontSize="16px"
        marginY="18px"
      >
        {props.description}
      </Typography>
    )
}

export default AdminDescription;