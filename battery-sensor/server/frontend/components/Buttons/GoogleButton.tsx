import React from "react";
import GoogleIcon from '@mui/icons-material/Google';
import { Button } from "@mui/material";

const GoogleButton = () => {
    return (
        <Button
            startIcon={<GoogleIcon />}
            className="mt-8"
            type="submit"
            name='provider'
            value='Google'
            variant="outlined"
            color="primary"
            style={{
                marginTop: "20px"
            }}
        >
            Logg inn med Google
        </Button>
    )
}

export default GoogleButton;
