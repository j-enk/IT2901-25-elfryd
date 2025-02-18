import { Box, IconButton } from "@mui/material";
import Link from "next/link";
import { useRouter } from "next/router";
import { useEffect, useState } from "react";
import KeyboardArrowDownIcon from "@mui/icons-material/KeyboardArrowDown";

type NavMenuLinkProps = {
  href: string;
  text: string;
  icon: React.ReactNode;
  isSubmenu?: boolean;
  hasSubmenu?: boolean;
  submenu?: React.ReactNode;
};

const NavMenuLink = (props: NavMenuLinkProps) => {
  const { href, text, icon, hasSubmenu, submenu, isSubmenu } = props;
  const [subMenuOpen, setSubMenuOpen] = useState(false);
  const router = useRouter();
  let isActive = router.pathname.split("/")[1] === href.split("/")[1];

  if (isSubmenu) {
    const { option } = router.query;
    if (option && option === href.split("/")[2]) isActive = true;
    else isActive = false;
  }

  const linkStyle = {
    backgroundColor: isActive ? "#0288d1" : "transparent",
    color: isActive ? "white" : "black",
    borderRadius: "4px",
    "&:hover": {
      backgroundColor: isActive ? "#0288d1" : "#e0e0e0",
    },
  };

  useEffect(() => {
    if (isActive) setSubMenuOpen(true);
  }, [isActive]);

  return (
    <>
      <Box
        display="flex"
        flexDirection="row"
        alignItems="center"
        gap="8px"
        py={1}
        px={0}
        sx={linkStyle}
      >
        <Box>{icon}</Box>
        <Link href={href}>{text}</Link>

        {hasSubmenu && (
          <IconButton
            sx={{
              py: 0,
              ml: "auto",
              transform: subMenuOpen ? "rotate(-180deg)" : "rotate(0)",
              transition: "transform 0.2s ease-in-out",
            }}
            onClick={() => setSubMenuOpen(!subMenuOpen)}
          >
            <KeyboardArrowDownIcon fontSize="medium" />
          </IconButton>
        )}
      </Box>
      {subMenuOpen && hasSubmenu && submenu}
    </>
  );
};

export default NavMenuLink;
