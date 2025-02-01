import { Box, IconButton, Tooltip, TooltipProps, styled, tooltipClasses } from "@mui/material";
import HelpOutlineIcon from '@mui/icons-material/HelpOutline';
import React from "react";

type CustomToolTipsProps = {
    title: string,
    description: string, 
    children: JSX.Element | JSX.Element[],
}

const CustomToolTips = (props:CustomToolTipsProps) => {
    return (
        <Box display="flex" flexDirection="row">
            {props.children}
        <CustomToolTip title={
            <React.Fragment>
                <b>{props.title}</b>
                <br/>
                {props.description}
            </React.Fragment>
            } placement="top-start" arrow>
            <IconButton>
                <HelpOutlineIcon/>
            </IconButton>
        </CustomToolTip>
        </Box>
    );
}

const CustomToolTip = styled(({ className, ...props }: TooltipProps) => (
    <Tooltip {...props} classes={{ popper: className }} />
  ))(({ theme }) => ({
    [`& .${tooltipClasses.tooltip}`]: {
      backgroundColor: '#ffffff',
      color: 'rgba(0, 0, 0, 0.87)',
      maxWidth: 220,
      fontSize: theme.typography.pxToRem(12),
      border: '2px solid #0288d1',
    },
  }));


export default CustomToolTips;