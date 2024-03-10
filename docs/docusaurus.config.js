// @ts-check
// Note: type annotations allow type checking and IDEs autocompletion

const lightCodeTheme = require('prism-react-renderer/themes/github');
const darkCodeTheme = require('prism-react-renderer/themes/dracula');

/** @type {import('@docusaurus/types').Config} */
const config = {
  title: 'NemesisDB',
  tagline: '',
  favicon: 'img/favicon.ico',
  
  // Set the production url of your site here
  url: 'https://docs.nemesisdb.io',

  baseUrl: '/',

  // GitHub pages deployment config.
  organizationName: 'nemesisdb', // Usually your GitHub org/user name.
  projectName: 'docs-deploy', // Usually your repo name.

  onBrokenLinks: 'throw',
  onBrokenMarkdownLinks: 'warn',

  // Even if you don't use internalization, you can use this field to set useful
  // metadata like html lang. For example, if your site is Chinese, you may want
  // to replace "en" with "zh-Hans".
  i18n: {
    defaultLocale: 'en',
    locales: ['en'],
  },

  presets: [
    [
      'classic',
      /** @type {import('@docusaurus/preset-classic').Options} */
      ({
        docs:
        {
          routeBasePath: '/',
          sidebarPath: require.resolve('./sidebars.js'),
        },
        blog: false,
        // blog: {
        //   showReadingTime: true,
        //   // Please change this to your repo.
        //   // Remove this to remove the "edit this page" links.
        //   editUrl:
        //     'https://github.com/facebook/docusaurus/tree/main/packages/create-docusaurus/templates/shared/',
        // },
        theme:
        {
          customCss: require.resolve('./src/css/custom.css'),
        },
      }),
    ],
  ],

  themeConfig:
    /** @type {import('@docusaurus/preset-classic').ThemeConfig} */
    ({
      // Replace with your project's social card
      image: 'img/docusaurus-social-card.jpg',
      navbar: {
        title: 'Docs',
        logo: {
          alt: 'NemesisDB',
          src: 'img/logo_nemesis_200.png',
        },
        items: [
          {
            type: 'docSidebar',
            sidebarId: 'apiSidebar',
            position: 'left',
            label: 'API',            
          },
          {
            type: 'docSidebar',
            sidebarId: 'tutorialSidebar',
            position: 'left',
            label: 'Tutorials',
          },
        ],
      },
      colorMode:
      {
        defaultMode: 'dark',
        disableSwitch: false,        
        respectPrefersColorScheme: false,
        darkCodeTheme: darkCodeTheme,
      },
      footer:
      {
        style: 'dark',
        links:
        [
          {
            title: 'Community',
            items:
            [
              {
                label: 'LinkedIn',
                href: 'https://www.linkedin.com/company/nemesisdb',
              },
              {
                label: 'Stack Overflow',
                href: 'https://stackoverflow.com/questions/tagged/nemesisdb',
              },
              {
                label: 'Twitter',
                href: 'https://twitter.com/nmsisdb',
              },
            ],
          },          
        ],
        copyright: `Copyright © ${new Date().getFullYear()} NemesisDB LTD.`,
      },
      prism:
      {
        additionalLanguages: ['bash', 'json'],
        theme: darkCodeTheme      
      },
    }),
};


module.exports = config;
