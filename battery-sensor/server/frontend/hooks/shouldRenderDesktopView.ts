import { useMediaQuery } from "@mui/material";

const useRenderDesktopView = () => {
    // Hook used to determine whether the user device is mobile
    // used to render different components depending on mobile or desktop
    const isOnMbile = useMediaQuery("(max-width: 760px)");
    return !isOnMbile;
};

export default useRenderDesktopView;