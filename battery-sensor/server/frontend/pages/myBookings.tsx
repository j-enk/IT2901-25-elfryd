import { Box } from "@mui/material";
import styles from "../styles/Home.module.css";
import NavBar from "../components/Home/NavBar";
import DesktopSideBar from "../components/Home/DesktopSideBar";
import UnlockComponent from "../components/Home/UnlockComp";
import useRenderDesktopView from "../hooks/shouldRenderDesktopView";
import { Dispatch, useEffect, useState } from "react";
import MyBookingsComponent from "../components/myBookings/MyBookingsComponent";
import { getProfile } from "../features/user/userSlice";
import { useAppDispatch } from "../features/hooks";
import { useSelector } from "react-redux";
import { selectUserStatus } from "../features/user/userSelector";

const actionDispatch = (dispatch: Dispatch<any>) => ({
    fetchProfile: () => dispatch(getProfile()),
});

function MyBookings() {

    const profileStatus = useSelector(selectUserStatus);

    const { fetchProfile } =
        actionDispatch(useAppDispatch());

    const [unlockOpen, setUnlockOpen] = useState(false);

    useEffect(() => {
        if (profileStatus === "idle") {
            fetchProfile();
        }
    }, []);

    function handleShow() {
        setUnlockOpen((wasOpen) => !wasOpen);
    }

    const shouldRenderDesktopView = useRenderDesktopView();

    return (
        <>
            <main className={styles.main}>
                <NavBar handleOpenShow={handleShow} showWeather={shouldRenderDesktopView} />
                <Box display="flex" flexDirection="row">
                    {shouldRenderDesktopView && <DesktopSideBar />}
                    <MyBookingsComponent handleOpenShow={handleShow} />

                </Box>
                <UnlockComponent handleShow={handleShow} open={unlockOpen} />
            </main>
        </>
    )
}

export default MyBookings;